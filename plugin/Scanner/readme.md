## ����ɨ����ģ��

nginx.conf�����ļ���������
```
        location /faro/ {
		add_header Access-Control-Allow-Origin *;
			add_header Access-Control-Allow-Headers X-Requested-With;
			add_header Access-Control-Allow-Methods GET,POST,OPTIONS;
			proxy_pass   http://172.17.9.20/;
        }
```
