static void game_log(const char* text, ...)
{
    va_list argp;
    va_start(argp, text);
    vprintf(text, argp);
    va_end(argp);
    fflush(stdout);
}
