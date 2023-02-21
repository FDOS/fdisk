segment INT24_TEXT class=CODE ALIGN=2   

  global _int24_handler

_int24_handler:
        xor ax, ax
        iret
