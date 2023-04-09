/*
	Código Js para ficar atualizando os valores medidos
	a cada X segundos
*/

var elemTemp = document.getElementById("temp");	// Pega a tag que mostra a temperatura
var elemUmid = document.getElementById("umid"); // Pega a tag que mostra a iluminação
var elemIlum = document.getElementById("ilum"); // Pega a tag que mostra a umidade
var reqDict;	// Variável que recebe os valores da requisição HTTP

function reqListener(){
	// Função para obter a resposta da requisição e atualizar os valores no site
	reqDict = JSON.parse(this.responseText);
	elemTemp.innerHTML = reqDict['TEMP'];
	elemIlum.innerHTML = reqDict['ILUM'];
	elemUmid.innerHTML = reqDict['UMID'];
}

var oReq = new XMLHttpRequest();
oReq.addEventListener("load", reqListener);
function requestInfo(){
	// Função para enviar requisição HTTP para dados
	oReq.open("GET", "127.0.0.1?data=1");
	oReq.send();
}

requestInfo();
setInterval(requestInfo, 2000);	// Executa a função requestInfo a cada 5000 ms




/*
function requestInfo(){
	//elemTemp += 1;
	if(elemTemp.textContent === '99'){
		elemTemp.innerHTML = '0'
	} else{
		elemTemp.innerHTML = (parseFloat(elemTemp.textContent) + 1).toString()
	}
}
*/