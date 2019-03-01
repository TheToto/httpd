#!/bin/sh

if test ! -d env ; then
    python -m venv env
    source env/bin/activate
    pip install -r tests/requirements.txt
else
    source env/bin/activate
fi

pytest -vv

deactivate

killall spider
exit 0
