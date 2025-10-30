
# Robô Explorador Espacial com ESP32 e IoT

Este repositório contém o projeto completo de um robô explorador IoT, controlado remotamente via Wokwi e equipado com sensores para analisar o ambiente, simulando a busca por condições de vida em outro planeta. Os dados coletados são enviados para um backend Python (Flask) e armazenados em um banco de dados SQLite.

## 🎯 Objetivo do Projeto

Este projeto é dividido em quatro etapas principais:

1.  **Controle Remoto (Simulado):** Criação de um controle remoto virtual no Wokwi usando um ESP32 e um Joystick para enviar comandos de movimento.
2.  **Robô Físico:** Montagem de um robô físico com ESP32, motores, e um conjunto de sensores (Temperatura, Umidade, Luz, Presença) para coletar dados do ambiente.
3.  **Backend e Banco de Dados:** Desenvolvimento de uma API em Python (Flask) para receber os dados dos sensores via HTTP POST e armazená-los em um banco de dados SQLite.
4.  **Integração:** Conexão de todas as partes, onde o controle comanda o robô (via MQTT) e o robô reporta seus dados para o backend.

## 🛠️ Tecnologias Utilizadas

  * **Hardware (Robô Físico):**
      * ESP32
      * Sensores: DHT11 (Temperatura e Umidade), LDR (Luminosidade), PIR (Presença)
      * Motores DC (com driver ou Servos de Rotação Contínua)
      * LEDs (Verde e Vermelho)
  * **Simulação (Controle):**
      * [Wokwi](https://wokwi.com/)
      * ESP32 (virtual)
      * Joystick Analógico
  * **Backend & Banco de Dados:**
      * Python 3
      * Flask (Servidor/API)
      * SQLite3 (Banco de Dados)
  * **Comunicação:**
      * MQTT (Comunicação Controle -\> Robô)
      * HTTP (Comunicação Robô -\> Backend)
      * Callmebot (Alertas via WhatsApp)
  * **Ambiente de Desenvolvimento:**
      * Visual Studio Code
      * PlatformIO IDE (Para programação do ESP32)

## 📁 Estrutura do Repositório

O projeto está organizado da seguinte forma:

```
IOT-ESP32-ROBO-ESPACIAL/
│
├── backend/                 <-- Pasta do servidor
│   ├── .venv/               <-- Ambiente virtual do Python (ignorado)
│   ├── app.py               <-- Script do servidor (Flask + SQLite)
│   └── robo_explorador.db   <-- Banco de dados (criado pelo app.py)
│
├── firmware/                <-- Pasta dos códigos do ESP32
│   ├── controle.ino         <-- Código do controle remoto (Wokwi)
│   └── robo.ino             <-- Código do robô físico (PlatformIO)
│
└── README.md                <-- Este arquivo de documentação
```

## 🚀 Como Executar

Siga os passos abaixo para configurar e executar cada parte do projeto.

### Etapa 01: Controle Remoto no Wokwi

1.  Acesse [Wokwi.com](https://wokwi.com/).
2.  Crie um novo projeto com a placa ESP32, Joystick Analógico, LEDs e um Botão.
3.  Copie o código do arquivo `firmware/controle.ino` e cole no editor do Wokwi.
4.  Inicie a simulação. O controle começará a enviar comandos MQTT para o broker `test.mosquitto.org` no tópico `/robo/comandos`.

### Etapa 02 & 03: Robô Físico e Backend

Estas duas etapas devem ser executadas em conjunto. O robô (Cliente) precisa se conectar ao Backend (Servidor).

#### A. Configurando o Backend (Servidor Python)

O backend é responsável por criar o banco de dados e escutar por dados dos sensores.

1.  **Abra este projeto no VS Code.**
2.  **Abra um terminal** (`Terminal` \> `Novo Terminal` ou `Ctrl + '`).
3.  **Navegue até a pasta `backend`:**
    ```bash
    cd backend
    ```
4.  **Crie um ambiente virtual** (dentro da pasta `backend`):
    ```bash
    py -m venv .venv
    ```
5.  **Ative o ambiente virtual:**
    ```bash
    .\.venv\Scripts\activate
    ```
6.  **Instale as dependências** (apenas Flask):
    ```bash
    py -m pip install Flask
    ```
7.  **Execute o servidor:**
    ```bash
    py app.py
    ```
    *O terminal mostrará que o servidor está rodando em `http://0.0.0.0:5000/` e o arquivo `robo_explorador.db` será criado **dentro da pasta `backend`**.*

#### B. Configurando o Robô Físico (ESP32)

O robô se conecta ao Wi-Fi, escuta os comandos do Wokwi (via MQTT) e envia seus dados para o Backend (via HTTP).

1.  **Montagem:** Monte o circuito físico do robô conforme o esquemático (motores, sensores, LEDs).
2.  **Encontre o IP do seu computador:**
      * Deixe o Backend rodando (passo A-7).
      * Abra um *novo terminal* no VS Code e digite `ipconfig`.
      * Encontre seu **Endereço IPv4** (ex: `192.168.1.10`).
3.  **Atualize o código `robo.ino`:**
      * Abra o arquivo `firmware/robo.ino`.
      * Encontre a linha `const char* backend_server` (próximo ao topo).
      * **Substitua o IP de exemplo** pelo IP do seu computador encontrado no passo anterior:
        ```cpp

        const char* backend_server = "http://192.168.1.10:5000/leituras";
        ```
      * Atualize também suas credenciais do **Callmebot** (número e APIKey) no código.
4.  **Faça o Upload para o ESP32:**
      * Instale a extensão **PlatformIO IDE** no VS Code.
      * Conecte o ESP32 físico ao seu computador via USB.
      * Abra a pasta `firmware` (ou o arquivo `robo.ino`).
      * Clique no ícone de seta (→) **"PlatformIO: Upload"** na barra de status azul (canto inferior esquerdo).
5.  **Monitore:**
      * Após o upload, clique no ícone de tomada (🔌) **"PlatformIO: Serial Monitor"**.
      * O robô pedirá para você selecionar a rede Wi-Fi e digitar a senha.
      * **IMPORTANTE:** O robô e o computador (Backend) devem estar na **mesma rede Wi-Fi**.

### Etapa 04: Consultando os Dados Salvos

Após o robô começar a enviar dados, você pode verificá-los de duas formas:

1.  **Pelo Navegador (API GET):**

      * Enquanto o backend (`py app.py`) estiver rodando, abra no seu navegador:
      * `http://localhost:5000/leituras`
      * Você verá um JSON com as 100 leituras mais recentes.

2.  **Pelo VS Code (SQLite Explorer):**

      * Instale a extensão **"SQLite"** (de `alexcvzz` ou similar).
      * Pressione `Ctrl + Shift + P` e escolha `SQLite: Open Database`.
      * Selecione o arquivo `backend/robo_explorador.db`.
      * Um painel "SQLITE EXPLORER" aparecerá na barra lateral, mostrando sua tabela `leituras` e todas as colunas.