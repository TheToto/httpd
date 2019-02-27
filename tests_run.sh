#!/bin/sh

if test ! -d env ; then
    python -m venv env
fi

    source env/bin/activate
if test ! -d env ; then
    pip install -r requirements.txt
fi

pytest -vv

deactivate
