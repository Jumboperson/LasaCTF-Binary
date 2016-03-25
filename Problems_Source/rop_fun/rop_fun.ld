ENTRY(main)                /* the name of the entry label */

SECTIONS {
    . = 0x00100000;          /* the code should be loaded at 1 MB */

    .rop ALIGN (0x1000) :   /* align at 4 KB */
    {
        *(.rop)             /* all text sections from all files */
    }

    .phun ALIGN (0x1000) : /* align at 4 KB */
    {
        *(.phun)          /* all read-only data sections from all files */
    }

    .stk ALIGN (0x1000) :   /* align at 4 KB */
    {
        *(.stk)             /* all data sections from all files */
    }
}