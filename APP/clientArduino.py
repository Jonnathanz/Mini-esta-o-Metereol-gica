import socket

def client(host='192.168.100.200', port=8082):
	data_payload = 2048
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	server_address = (host, port)

	sock.connect(server_address)	# 4º Etapa: Connect

	try:
		message = "Ola servidor, sou o cliente"
		sock.sendall(message.encode('utf-8'))	# 5º Etapa: Send
		
		data = sock.recv(data_payload)			# 6º Etapa: Send
		print("Data: {}".format(data[6:-1].decode()))
	except socket.error as e:
		print("Socket error: {}".format(e))
	except Exception as e:
		print("Other exception: {}".format(e))

	sock.close()						# 7º Etapa: Close
	return data[6:-1].decode() if type(data[6:-1].decode())	== dict else False				

if __name__ == '__main__':
	client()