@echo off 

if "%1" == "apply" (
    terraform %1 -auto-approve
) else (
    if "%1" == "destroy" (
    terraform %1 -auto-approve
    ) else (
        terraform %*
    )
)
PAUSE
