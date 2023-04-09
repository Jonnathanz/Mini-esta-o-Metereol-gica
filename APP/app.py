import time
import socket
import threading

'''
	socket.socket: Criaçao do socket com determinadas características
		AF_INET: Endereços de rede IPV4
		SOCK_STREAM: Protocolo TCP

	setsockopt: Definiçoes de opçoes de socket
		SOL_SOCKET:
		SO_REUSEADDR: allows a socket to forcibly bind to a port in use by another socket.

	bind:  method is used to associate the socket with a specific network interface and port number

	listen: enables a server to accept connections.

	accept:

		execution and waits for an incoming connection. When a client connects, 
		it returns a new socket object representing the connection and a tuple holding the address of the client. 

	recv: This reads whatever data the client sends and echoes it back using

	send: Send data to the socket. The socket must be connected to a remote socket. 
''' 

def serverHTTP(host="localhost", port=8080):
	data_payload = 2048
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
	sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	server_address = (host, port)
	sock.bind(server_address)		# 2º Etapa: Bind
	sock.listen()					# 3º Etapa: Listen
	while True:
		client, address = sock.accept()
		conn = client.recv(data_payload)
		conn_utf8 = conn.decode('utf-8')
		if "GET / HTTP/1.1" in conn_utf8:
			# ENVIA O DOC HTML
			html_file = open("pag.htm",  "r", encoding="utf-8")
			client.sendall(bytearray("HTTP/1.1 200 ok\n", "utf-8"))
			client.sendall(bytearray("Content-Type: text/html\n charset=UTF-8\n", "utf-8"))
			client.sendall(bytearray("\n", "utf-8"))
			client.sendall(bytearray(html_file.read(), "utf-8"))
			client.sendall(bytearray("\n", "utf-8"))
			html_file.close()
		elif "GET /style.css HTTP/1.1" in conn_utf8:
			# ENVIA O DOC CSS
			css_file = open("style.css","r", encoding="utf-8")
			client.sendall(bytearray("HTTP/1.1 200 ok\n", "utf-8"))
			client.sendall(bytearray("Content-Type: text/css\n", "utf-8"))
			client.sendall(bytearray("\n", "utf-8"))
			client.sendall(bytearray(css_file.read(), "utf-8"))
			client.sendall(bytearray("\n", "utf-8"))
			css_file.close()
		elif "GET /script.js HTTP/1.1" in conn_utf8:
			# ENVIA O DOC JAVASCRIPT
			js_file = open("script.js","r", encoding="utf-8")
			client.sendall(bytearray("HTTP/1.1 200 ok\n", "utf-8"))
			client.sendall(bytearray("Content-Type: text/javascript\n", "utf-8"))
			client.sendall(bytearray("\n", "utf-8"))
			client.sendall(bytearray(js_file.read(), "utf-8"))
			client.sendall(bytearray("\n", "utf-8"))
			js_file.close()
		elif "GET /127.0.0.1?data=1 HTTP/1.1" in conn_utf8:
			#print(conn_utf8)
			client.sendall(bytearray("HTTP/1.1 200 ok\n", "utf-8"))
			client.sendall(bytearray("Content-Type: text\n", "utf-8"))
			client.sendall(bytearray("\n", "utf-8"))
			if data_send != None:
				client.sendall(bytearray(data_send.decode(), "utf-8"))
		else:
			#print("\n\n\nREQUISIÇÕES DESCONHECIDAS: \n\n")
			#print(conn_utf8)
			pass
		client.close()

def clientArduino(host='192.168.100.200', port=8082, loop=True):
	global data_send
	data_send = None
	data_payload = 2048
	__delay = 2
	while loop:	
		sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		server_address = (host, port)
		try:
			sock.connect(server_address)	# 4º Etapa: Connect
			message = "Ola servidor, sou o cliente"
			sock.sendall(message.encode('utf-8'))	# 5º Etapa: Send
			
			data = sock.recv(data_payload)			# 6º Etapa: Send
			#print("Data: {}".format(data[6:-1].decode()))

			data_send = data[6:-1]
			print(data_send)
		except socket.error as e:
			print("Socket error: {}".format(e))
		except Exception as e:
			print("Other exception: {}".format(e))

		sock.close()						# 7º Etapa: Close
		time.sleep(__delay)
	#return data[6:-1].decode() if type(data[6:-1].decode())	== dict else False

if __name__ == '__main__':
	thread1 = threading.Thread(target=serverHTTP)
	thread2 = threading.Thread(target=clientArduino)

	clientArduino(loop=False) # Para inicializar a variável global
	thread1.start()
	thread2.start()
	