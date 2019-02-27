#!/bin/sh

if test ! -d env ; then
    python -m venv env
    source env/bin/activate
    pip install -r requirements.txt
else
    source env/bin/activate
fi

pytest -vv

deactivate
