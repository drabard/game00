#include "gpu.h"

#define GPU_GL_ERROR_CHECK 1

static gpu_log_fptr text_log = NULL;

static void gpu_memcpy(void* dst, const void* src, uint32_t nbytes)
{
    uint8_t* d = dst;
    const uint8_t* s = src;
    for (; nbytes; --nbytes)
        *d++ = *s++;
}

void gpu_set_log(gpu_log_fptr l) { text_log = l; }

static GLenum check_gl_errors(const char* desc)
{
#if GPU_GL_ERROR_CHECK
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        text_log("ERROR [OpenGL]: Failure %s: ", desc);
        switch (err) {
        case GL_INVALID_ENUM: {
            text_log("GL_INVALID_ENUM\n");
        } break;
        case GL_INVALID_VALUE: {
            text_log("GL_INVALID_VALUE\n");
        } break;
        case GL_INVALID_OPERATION: {
            text_log("GL_INVALID_OPERATION\n");
        } break;
        case GL_STACK_OVERFLOW: {
            text_log("GL_STACK_OVERFLOW\n");
        } break;
        case GL_STACK_UNDERFLOW: {
            text_log("GL_STACK_UNDERFLOW\n");
        } break;
        case GL_OUT_OF_MEMORY: {
            text_log("GL_OUT_OF_MEMORY\n");
        } break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: {
            text_log("GL_INVALID_FRAMEBUFFER_OPERATION\n");
        } break;
        case GL_CONTEXT_LOST: {
            text_log("GL_CONTEXT_LOST\n");
        } break;
        }
        return err;
    }
#endif
    return GL_NO_ERROR;
}

enum gpu_status gpu_init()
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (check_gl_errors("initializing graphics") != GL_NO_ERROR) {
        return GPU_FAILURE;
    }

    return GPU_OK;
}

enum gpu_status gpu_link_program(GLuint* dst, GLuint vertex_shader,
                                 GLuint fragment_shader)
{
    GLuint p = glCreateProgram();

    glAttachShader(p, vertex_shader);
    glAttachShader(p, fragment_shader);
    if (check_gl_errors("glAttachShader") != GL_NO_ERROR)
        goto error;

    glLinkProgram(p);
    if (check_gl_errors("glLinkProgram") != GL_NO_ERROR)
        goto error;

    *dst = p;

    return GPU_OK;

error:
    return GPU_FAILURE;
}

enum gpu_status gpu_compile_shader(GLuint* dst, GLenum shader_type,
                                   const char* source)
{
    GLuint result;
    result = glCreateShader(shader_type);
    if (check_gl_errors("glCreateShader") != GL_NO_ERROR)
        goto error;

    glShaderSource(result, 1, &source, 0);
    if (check_gl_errors("glShaderSource") != GL_NO_ERROR)
        goto error;

    glCompileShader(result);
    if (check_gl_errors("glCompileShader") != GL_NO_ERROR)
        goto error;

    { // Check shader compilation
        GLint success;
        GLint log_len;
        char log_buf[1024];
        glGetShaderiv(result, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE) {
            glGetShaderiv(result, GL_INFO_LOG_LENGTH, &log_len);
            if (log_len > 1024)
                log_len = 1024;
            glGetShaderInfoLog(result, log_len, 0, log_buf);

            const char* shader_type_name;
            switch (shader_type) {
            case GL_VERTEX_SHADER: {
                shader_type_name = "vertex shader";
            } break;
            case GL_FRAGMENT_SHADER: {
                shader_type_name = "fragment shader";
            } break;
            default: {
                shader_type_name = "unknown shader";
            }
            }

            text_log("ERROR [%s compilation]:\n%s\n", shader_type_name, log_buf);
            return GPU_FAILURE;
        }
    }

    *dst = result;

    return GPU_OK;
error:
    return GPU_FAILURE;
}

enum gpu_status gpu_activate_program(gpu_program p)
{
    glUseProgram(p);
    
    if (check_gl_errors("glUseProgram") != GL_NO_ERROR)
        return GPU_FAILURE;

    return GPU_OK;
}

const uint32_t gpu_max_vert_bytes = 9 * sizeof(float);

uint8_t gpu_pack_verts(void* outbuf, float* positions, float* normals,
                       float* texcoords, uint32_t nverts)
{
    uint8_t res_flags = 0;
    uint32_t vert_nfloats = 0;

    if (positions) {
        res_flags |= GPU_POS;
        vert_nfloats += 3;
    }

    if (normals) {
        res_flags |= GPU_NORM;
        vert_nfloats += 3;
    }

    if (texcoords) {
        res_flags |= GPU_TEXCOORD;
        vert_nfloats += 3;
    }

    uint32_t start_offset = 0;
    if (positions) {
        float* o = outbuf;
        o += start_offset;
        for (uint32_t vert_i = 0; vert_i < nverts; ++vert_i) {
            gpu_memcpy(o, positions + (vert_i * 3), 3 * sizeof(float));
            o += vert_nfloats;
        }
        start_offset += 3;
    }

    if (normals) {
        float* o = outbuf;
        o += start_offset;
        for (uint32_t vert_i = 0; vert_i < nverts; ++vert_i) {
            gpu_memcpy(o, normals + (vert_i * 3), 3 * sizeof(float));
            o += vert_nfloats;
        }
        start_offset += 3;
    }

    if (texcoords) {
        float* o = outbuf;
        o += start_offset;
        for (uint32_t vert_i = 0; vert_i < nverts; ++vert_i) {
            gpu_memcpy(o, texcoords + (vert_i * 3), 3 * sizeof(float));
            o += vert_nfloats;
        }
        start_offset += 3;
    }

    return res_flags;
}

enum gpu_status gpu_vertex_buffer_create(struct gpu_vertex_buffer* buf, const void* vertices, uint8_t vert_flags, const uint32_t* indices, uint32_t nverts, uint32_t nindices)
{
    GLuint vao, vb, eb;

    glGenVertexArrays(1, &vao);
    if (check_gl_errors("glGenVertexArrays") != GL_NO_ERROR)
        goto error;

    glGenBuffers(1, &vb);
    glGenBuffers(1, &eb);
    if (check_gl_errors("glGenBuffers") != GL_NO_ERROR)
        goto error;

    uint32_t vert_nbytes = 0;
    if ((vert_flags & GPU_POS) != 0)
        vert_nbytes += (3 * sizeof(float));
    if ((vert_flags & GPU_NORM) != 0)
        vert_nbytes += (3 * sizeof(float));
    if ((vert_flags & GPU_TEXCOORD) != 0)
        vert_nbytes += (3 * sizeof(float));

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eb);
    if (check_gl_errors("binding state") != GL_NO_ERROR)
        goto error;

    glBufferData(GL_ARRAY_BUFFER, nverts * vert_nbytes, vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, nindices * sizeof(uint32_t), indices,
                 GL_STATIC_DRAW);
    if (check_gl_errors("glBufferData") != GL_NO_ERROR)
        goto error;

    float* stride = NULL;

    // 0: positions
    if ((vert_flags & GPU_POS) != 0) {
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vert_nbytes, 0);
        stride += 3;
    }

    // 1: normals
    if ((vert_flags & GPU_NORM) != 0) {
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vert_nbytes, stride);
        stride += 3;
    }

    // 2: texture coordinates
    if ((vert_flags & GPU_TEXCOORD) != 0) {
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, vert_nbytes, stride);
        stride += 3;
    }

    if (check_gl_errors("defining vertex attribute pointers") != GL_NO_ERROR)
        goto error;

    glBindVertexArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    buf->vao = vao;
    buf->vertex_buf = vb;
    buf->elem_buf = eb;
    buf->nindices = nindices;
    
    return GPU_OK;

error:
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vb);
    glDeleteBuffers(1, &eb);
    return GPU_FAILURE;
}

void gpu_vertex_buffer_destroy(struct gpu_vertex_buffer* buf)
{
    glDeleteVertexArrays(1, &buf->vao);
    glDeleteBuffers(1, &buf->vertex_buf);
    glDeleteBuffers(1, &buf->elem_buf);
}

void gpu_vertex_buffer_draw(const struct gpu_vertex_buffer* tgt)
{
    glBindVertexArray(tgt->vao);
    glDrawElements(GL_TRIANGLES, tgt->nindices, GL_UNSIGNED_INT, NULL);
    if (check_gl_errors("drawing vertex buffer") != GL_NO_ERROR)
        goto error;

    return;
    
error:
    text_log("ERROR: Failed to draw vertex buffer.\n");
}

enum gpu_status gpu_texture_create_RGBA(struct gpu_texture* dst, const uint8_t* data, uint32_t width, uint32_t height)
{
    GLuint t;
    glGenTextures(1, &t);

    glBindTexture(GL_TEXTURE_2D, t);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    if (check_gl_errors("creating RGBA texture") != GL_NO_ERROR) {
        return GPU_FAILURE;
    }

    dst->id = t;

    return GPU_OK;
}

enum gpu_status gpu_texture_create_R(struct gpu_texture* dst, const uint8_t* data, uint32_t width, uint32_t height)
{
    GLuint t;
    glGenTextures(1, &t);

    glBindTexture(GL_TEXTURE_2D, t);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED,
                 GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    if (check_gl_errors("creating R texture") != GL_NO_ERROR) {
        return GPU_FAILURE;
    }

    dst->id = t;

    return GPU_OK;
}

void gpu_texture_destroy(struct gpu_texture* tgt) { glDeleteTextures(1, &tgt->id); }

enum gpu_status gpu_get_uniform(gpu_program program, const char* name,
                                gpu_uniform* uniform)
{
    GLint res = glGetUniformLocation(program, name);
    if (res == -1)
        goto error;

    if (check_gl_errors("getting uniform location") != GL_NO_ERROR)
        goto error;

    *uniform = res;

    return GPU_OK;
    
error:
    text_log("ERROR: Could not retrieve uniform location for uniform %s.\n", name);
    return GPU_FAILURE;
}

enum gpu_status gpu_set_uniform_i(gpu_uniform uniform, int32_t value)
{
    glUniform1i(uniform, value);

    if (check_gl_errors("setting uniform value") != GL_NO_ERROR)
        return GPU_FAILURE;

    return GPU_OK;
}

enum gpu_status gpu_set_uniform_2f(gpu_uniform uniform, const float* values)
{
    glUniform2fv(uniform, 1, values);

    if (check_gl_errors("setting uniform value") != GL_NO_ERROR)
        return GPU_FAILURE;

    return GPU_OK;
}

enum gpu_status gpu_set_uniform_3f(gpu_uniform uniform, const float* values)
{
    glUniform3fv(uniform, 1, values);

    if (check_gl_errors("setting uniform value") != GL_NO_ERROR)
        return GPU_FAILURE;

    return GPU_OK;
}


enum gpu_status gpu_set_uniform_m4(gpu_uniform uniform, const float* values)
{
    glUniformMatrix4fv(uniform, 1, GL_FALSE, values);

    if (check_gl_errors("setting uniform value") != GL_NO_ERROR)
        return GPU_FAILURE;

    return GPU_OK;
}


enum gpu_status gpu_texture_bind(struct gpu_texture* tex, const gpu_program* programs,
                             uint32_t nprograms, uint32_t tex_unit,
                             const char* sampler_name)
{
    glActiveTexture(GL_TEXTURE0 + tex_unit);
    glBindTexture(GL_TEXTURE_2D, tex->id);

    for (uint32_t i = 0; i < nprograms; i++) {
        glUseProgram(programs[i]);
        GLint loc = glGetUniformLocation(programs[i], sampler_name);
        glUniform1i(loc, tex_unit);
    }

    if (check_gl_errors("binding texture") != GL_NO_ERROR)
        return GPU_FAILURE;

    return GPU_OK;
}

enum gpu_status gpu_uniform_buf_create(struct gpu_uniform_buf* ub,
                                       GLuint bind_location, const char* name,
                                       void* data, uint32_t nbytes)
{
    GLuint id;

    glGenBuffers(1, &id);
    if (check_gl_errors("creating uniform buffer") != GL_NO_ERROR)
        goto error;

    glBindBuffer(GL_UNIFORM_BUFFER, id);
    glBindBufferBase(GL_UNIFORM_BUFFER, bind_location, id);
    if (check_gl_errors("binding uniform buffer state") != GL_NO_ERROR)
        goto error;

    glBufferData(GL_UNIFORM_BUFFER, nbytes, data, GL_DYNAMIC_DRAW);
    if (check_gl_errors("initializing uniform buffer data") != GL_NO_ERROR)
        goto error;

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    ub->name = name;
    ub->buf = id;
    ub->bind_loc = bind_location;

    return GPU_OK;

error:
    return GPU_FAILURE;
}

void gpu_uniform_buf_destroy(struct gpu_uniform_buf* ub)
{
    glBindBuffer(GL_UNIFORM_BUFFER, ub->buf);
    glBindBufferBase(GL_UNIFORM_BUFFER, ub->bind_loc, 0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glDeleteBuffers(1, &ub->buf);
}

enum gpu_status gpu_uniform_buf_update(struct gpu_uniform_buf* ub, void* data,
                                       uint32_t nbytes)
{
    glBindBuffer(GL_UNIFORM_BUFFER, ub->buf);
    if (check_gl_errors("binding uniform buffer") != GL_NO_ERROR)
        goto error;

    GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    if (check_gl_errors("mapping uniform buffer") != GL_NO_ERROR)
        goto error;

    if (p == NULL)
        return GPU_FAILURE;

    gpu_memcpy(p, data, nbytes);

    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    return GPU_OK;

error:
    return GPU_FAILURE;
}

enum gpu_status gpu_uniform_buf_bind(struct gpu_uniform_buf* ub,
                                     const GLuint* programs, uint32_t nprograms)
{
    const char* name = ub->name;
    for (uint32_t prog_i = 0; prog_i < nprograms; ++prog_i) {
        GLuint p = programs[prog_i];
        GLuint block_i = glGetUniformBlockIndex(p, name);
        if (block_i == GL_INVALID_INDEX) {
            text_log(
                "ERROR: Failed to bind uniform block \"%s\" to shader program.\n",
                name);
            goto error;
        }
        glUniformBlockBinding(p, block_i, ub->bind_loc);
        if (check_gl_errors("binding uniform block to shader") != GL_NO_ERROR)
            goto error;
    }

    return GPU_OK;

error:
    return GPU_FAILURE;
}
