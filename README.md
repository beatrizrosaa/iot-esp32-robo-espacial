# iot-esp32-robo-espacial

#Componentes Necessários Para a montagem do circuito físico, você precisará dos seguintes componentes:

1x ESP32 
2x Motores DC 
1x Sensor de Temperatura 
1x Fotorresistor 
1x Sensor de presença (PIR) 
1x LED Verde 
1x LED Vermelho

#Instruções de Montagem

1.Conecte todos os componentes listados ao ESP32  seguindo o esquema elétrico definido para o projeto.

2.Certifique-se de verificar a polaridade correta dos motores para identificar o lado esquerdo e o direito.

3.Posicione os sensores (Temperatura, Fotorresistor e PIR) de forma que consigam captar os sinais do ambiente adequadamente.

4.Carregue o código do firmware do robô (arquivo .ino da feat/robo-lab ) no ESP32 utilizando a Arduino IDE.

#Como Rodar o Backend Python

#1.Pré-requisitos

Python 3 instalado.
Instale as bibliotecas Python necessárias (ex: Flask ou FastAPI e uvicorn).
# Exemplo para Flask
pip install Flask
# Exemplo para FastAPI
pip install fastapi uvicorn

#2.Configuração do Banco de Dados

Localize o arquivo de criação do banco de dados (ex: schema.sql) no repositório.
Execute este script para criar o banco de dados SQLite e a tabela leituras com a estrutura correta (id, timestamp, temperatura_c, umidade_pct, luminosidade, presenca, probabilidade_vida).

#3.Execução

# Exemplo para Flask
python app.py
# Exemplo para FastAPI
uvicorn main:app --reload

