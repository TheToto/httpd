#!/bin/sh

mkdir tests/garbage
if test ! -d env ; then
    python -m venv env
    source env/bin/activate
    pip install -r tests/requirements.txt
else
    source env/bin/activate
fi

pytest -vv

deactivate

pkill spider
rm tests/garbage -rf
exit 0
