# Spider Web Server

## Build
```
./run
```

## Run tests
```
./run check
```

## Launch
```
./spider [-t] config.json
```
"-t" arg is used to check config files. It will return a non-zero value
if the config is malformed.

## Bonus
### 301 redirections
-> In `config.json` file, inside vhost object :
```
    "old_uri_perm" :
    {
        "/old_path" : "/new_path",
        "/other_old_path" : "/new_path"
    }
```

## Authors
* thomas.lupin
* louis.holleville
* arthur.busuttil
* geraud.magne
