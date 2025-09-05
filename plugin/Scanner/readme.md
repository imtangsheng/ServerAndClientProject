## 法如扫描仪模块

nginx.conf配置文件代理设置
```
        location /faro/ {
		add_header Access-Control-Allow-Origin *;
			add_header Access-Control-Allow-Headers X-Requested-With;
			add_header Access-Control-Allow-Methods GET,POST,OPTIONS;
			proxy_pass   http://172.17.9.20/;
        }
```

## 网络http指令
### post 方法 查询序列号
```
http://192.200.1.20/faro/lswebapi/scanner-infos

{
    "_links": {
        "related": {
            "href": "/lswebapi/scanner-infos/internal-scanner-infos",
            "name": "internalScannerInfos"
        },
        "self": {
            "href": "/lswebapi/scanner-infos"
        }
    },
    "apiVersion": "1.2.0",
    "firmware-version": "6.8.1.6515",
    "licenses": [],
    "model": "Focus S Plus 350 A",
    "name": "LLS082016679",
    "serial-number": "LLS082016679"
}

```
### 关机指令
```
http://192.200.1.20/faro/lswebapi/operations/shutdown

<!DOCTYPE html>
<html>

<head>
	<title>Error</title>
	<style>
		html {
			color-scheme: light dark;
		}

		body {
			width: 35em;
			margin: 0 auto;
			font-family: Tahoma, Verdana, Arial, sans-serif;
		}
	</style>
</head>

<body>
	<h1>An error occurred.</h1>
	<p>Sorry, the page you are looking for is currently unavailable.<br/>
Please try again later.</p>
		<p>If you are the system administrator of this resource then you should check
			the error log for details.</p>
		<p><em>Faithfully yours, nginx.</em></p>
</body>

</html>
```


## issue
### sdk 奔溃
0xc0000005 错误通常与内存访问问题相关，可能是由于软件冲突、驱动程序问题、内存损坏或系统文件损坏等原因引起的。