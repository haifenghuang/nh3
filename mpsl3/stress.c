#include "mpsl.h"

void mpsl_disasm(mpdm_t prg);

int main(int argc, char *argv[])
{
    mpdm_t prg;
    wchar_t *ptr;

    mpdm_startup();

    ptr = L"this.x = 0; while (n > 0) { n = n - 1; } mp.init(); sub sum(a, b) { return a + b; } global v1, v2, v3 = {}, v4; sum(1, 2);";

    prg = mpsl_compile(MPDM_LS(ptr));
    mpsl_disasm(mpdm_aget(prg, 1));

    mpdm_dump(mpdm_exec(mpsl_compile(MPDM_LS(L"2 + 3;")), NULL, NULL));

    return 0;
}
