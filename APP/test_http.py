from time import time

delay_time = 5
start_time = time()
while True:
    end_time = time()
    #print(start_time - end_time)
    if end_time - start_time > delay_time:
        start_time = end_time
        print("ATIVADO")

'''
import re

a = b'GET / HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: keep-alive\r\nsec-ch-ua: " Not A;Brand";v="99", "Chromium";v="101", "Google Chrome";v="101"\r\nsec-ch-ua-mobile: ?0\r\nsec-ch-ua-platform: "Windows"\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/101.0.4951.67 Safari/537.36\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\nSec-Fetch-Site: none\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-User: ?1\r\nSec-Fetch-Dest: document\r\nAccept-Encoding: gzip, deflate, br\r\nAccept-Language: pt-BR,pt;q=0.9,en-US;q=0.8,en;q=0.7\r\n\r\n'
b = b'GET /style.css HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: keep-alive\r\nsec-ch-ua: " Not A;Brand";v="99", "Chromium";v="101", "Google Chrome";v="101"\r\nsec-ch-ua-mobile: ?0\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/101.0.4951.67 Safari/537.36\r\nsec-ch-ua-platform: "Windows"\r\nAccept: text/css,*/*;q=0.1\r\nSec-Fetch-Site: same-origin\r\nSec-Fetch-Mode: no-cors\r\nSec-Fetch-Dest: style\r\nReferer: http://127.0.0.1/\r\nAccept-Encoding: gzip, deflate, br\r\nAccept-Language: pt-BR,pt;q=0.9,en-US;q=0.8,en;q=0.7\r\n\r\n'
c = b'GET /favicon.ico HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: keep-alive\r\nsec-ch-ua: " Not A;Brand";v="99", "Chromium";v="101", "Google Chrome";v="101"\r\nsec-ch-ua-mobile: ?0\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/101.0.4951.67 Safari/537.36\r\nsec-ch-ua-platform: "Windows"\r\nAccept: image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8\r\nSec-Fetch-Site: same-origin\r\nSec-Fetch-Mode: no-cors\r\nSec-Fetch-Dest: image\r\nReferer: http://127.0.0.1/\r\nAccept-Encoding: gzip, deflate, br\r\nAccept-Language: pt-BR,pt;q=0.9,en-US;q=0.8,en;q=0.7\r\n\r\n'

a2 = a.decode('utf-8')
b2 = b.decode('utf-8')
c2 = c.decode('utf-8')
'''
