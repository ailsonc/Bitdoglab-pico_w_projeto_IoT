const express = require('express');
const mysql = require('mysql2');
const app = express();
app.use(express.json());

const db = mysql.createConnection({
    host: 'db',
    user: 'root',
    password: '123456',
    database: 'picoIoT'
});

db.connect(err => {
    if (err) {
        console.error('Erro ao conectar ao MySQL (Tentando novamente em breve):', err.message);
    } else {
        console.log('Conectado ao Banco de Dados MySQL!');
    }
});

// 1. GET - Buscar configuração completa pelo modelo
app.get('/configuracao/:modelo', (req, res) => {
    const query = 'SELECT nome_cor, r_ref, g_ref, b_ref FROM configuracao_cores WHERE modelo_celular = ?';
    db.query(query, [req.params.modelo], (err, results) => {
        if (err) return res.status(500).json({ erro: err.message });
        if (results.length === 0) return res.status(404).send('Modelo não cadastrado');
        res.json(results[0]);
    });
});

// 2. POST - Salvar validação (O banco recusará se status não for PASS ou FAIL)
app.post('/validacao', (req, res) => {
    const { modelo, numero_serie, status } = req.body;
    const query = 'INSERT INTO historico_validacao (modelo, numero_serie, status) VALUES (?, ?, ?)';
    
    db.query(query, [modelo, numero_serie, status], (err) => {
        if (err) {
            // Se o status for inválido, o MySQL retornará erro de data truncated/enum
            return res.status(400).json({ erro: "Dados inválidos ou Status incorreto (Use PASS/FAIL)" });
        }
        res.status(201).send('Validação registrada com sucesso');
    });
});

// 3. GET - Ver histórico de validações
app.get('/historico', (req, res) => {
    const query = 'SELECT * FROM historico_validacao ORDER BY data_hora DESC';
    db.query(query, (err, results) => {
        if (err) return res.status(500).send(err);
        res.json(results);
    });
});

app.listen(3000, () => console.log('API ativa no container Docker na porta 3000'));