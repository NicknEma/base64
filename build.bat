@echo off
del *.pdb > NUL 2> NUL
del *.rdi > NUL 2> NUL
cl base64.c             -nologo -Fe:base64.exe             -Z7 -link -incremental:no -opt:ref
cl base64_experiments.c -nologo -Fe:base64_experiments.exe -Z7 -link -incremental:no -opt:ref
del *.ilk > NUL 2> NUL
del *.obj > NUL 2> NUL
