import sqlite3
from flask import Flask, request, jsonify
from datetime import datetime

# Cria a aplicação Flask
app = Flask(__name__)

# --- Configuração do Banco de Dados  ---
DATABASE_NAME = 'robo_explorador.db'

def get_db():
    """Conecta ao banco de dados SQLite."""
    conn = sqlite3.connect(DATABASE_NAME)
    conn.row_factory = sqlite3.Row # Isso permite acessar as colunas pelo nome
    return conn

def init_db():
    """Cria a tabela no banco de dados se ela não existir."""
    print("Iniciando o banco de dados...")
    try:
        with app.app_context():
            db = get_db()
            cursor = db.cursor()
            # [cite: 228, 229, 230, 232, 233, 235, 237, 238]
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS leituras (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp TEXT NOT NULL,
                    temperatura_c REAL,
                    umidade_pct REAL,
                    luminosidade INTEGER,
                    presenca INTEGER,
                    probabilidade_vida REAL
                )
            ''')
            db.commit()
            print("Tabela 'leituras' verificada/criada com sucesso.")
    except Exception as e:
        print(f"Erro ao inicializar o banco de dados: {e}")
    finally:
        if db:
            db.close()

# --- Rota para receber os dados do Robô (POST)  ---
@app.route('/leituras', methods=['POST'])
def adicionar_leitura():
    try:
        # Pega os dados JSON enviados pelo ESP32 
        dados = request.get_json()

        # Extrai os dados
        temp = dados.get('temperatura_c')
        umid = dados.get('umidade_pct')
        luz = dados.get('luminosidade')
        pres = dados.get('presenca')
        prob = dados.get('probabilidade_vida')
        
        # Gera o timestamp no servidor
        timestamp = datetime.now().isoformat()

        # Insere no banco de dados
        db = get_db()
        cursor = db.cursor()
        cursor.execute(
            '''
            INSERT INTO leituras (timestamp, temperatura_c, umidade_pct, luminosidade, presenca, probabilidade_vida)
            VALUES (?, ?, ?, ?, ?, ?)
            ''',
            (timestamp, temp, umid, luz, pres, prob)
        )
        db.commit()
        db.close()

        print(f"Dados recebidos e salvos: {dados}")
        return jsonify({"status": "sucesso", "dados_recebidos": dados}), 201
    
    except Exception as e:
        print(f"Erro ao processar requisição: {e}")
        return jsonify({"status": "erro", "mensagem": str(e)}), 400

# --- Rota para consultar os dados (GET)  ---
@app.route('/leituras', methods=['GET'])
def obter_leituras():
    try:
        db = get_db()
        cursor = db.cursor()
        
        # Busca as últimas 100 leituras, da mais recente para a mais antiga
        cursor.execute("SELECT * FROM leituras ORDER BY id DESC LIMIT 100")
        leituras = [dict(row) for row in cursor.fetchall()]
        db.close()
        
        return jsonify(leituras)
    except Exception as e:
        print(f"Erro ao buscar leituras: {e}")
        return jsonify({"status": "erro", "mensagem": str(e)}), 500

# --- Ponto de entrada para rodar o servidor ---
if __name__ == '__main__':
    init_db() # Inicializa o banco de dados antes de rodar o app
    # '0.0.0.0' faz o servidor ser acessível por outros dispositivos na sua rede (o ESP32)
    app.run(host='0.0.0.0', port=5000, debug=True)