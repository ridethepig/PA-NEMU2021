.globl  _start
_start:
  move sp, a0 # after stack restrore, a0 stores user stack top
  # i think a0 just stores argc position
  move s0, zero
  jal call_main
