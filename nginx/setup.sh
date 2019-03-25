MY_PATH=`dirname "$0"`
MY_PATH=`( cd "$MY_PATH" && pwd )`

rm -rf /tmp/nginx-1.14.2/ &> /dev/null
rm -rf /tmp/nginx/
cd /tmp
echo "Dowloading nginx..."
wget https://nginx.org/download/nginx-1.14.2.tar.gz &> /dev/null
tar xvf nginx-1.14.2.tar.gz &> /dev/null
cd nginx-1.14.2/
C_NGINX=/tmp/nginx/
mkdir ${C_NGINX}
mkdir ${C_NGINX}/log
mkdir ${C_NGINX}/body
echo "Configure nginx..."
./configure --conf-path=${C_NGINX}/nginx.conf --sbin-path=${C_NGINX}/nginx --pid-path=${C_NGINX}/nginx.pid --error-log-path=${C_NGINX}/error.log --lock-path=${C_NGINX}/nginx.lock --user=epita --group=epita --http-log-path=${C_NGINX}/log/access.log --http-client-body-temp-path=${C_NGINX}/body/ --http-proxy-temp-path=/tmp/ --prefix=${C_NGINX} &> /dev/null
echo "Make nginx..."
make -j9 &> /dev/null
cp objs/nginx ${C_NGINX}/nginx
cd ${C_NGINX}
echo "Launching nginx !"
cp $MY_PATH/nginx.conf .
./nginx
