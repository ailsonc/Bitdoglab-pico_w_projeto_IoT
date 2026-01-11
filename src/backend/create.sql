-- Tabela de configurações por modelo
CREATE TABLE configuracao_cores (
    id INT AUTO_INCREMENT PRIMARY KEY,
    modelo_celular VARCHAR(100) NOT NULL,
    nome_cor VARCHAR(50) NOT NULL, -- Ex: 'Azul', 'Branco' 
    r_ref INT NOT NULL,
    g_ref INT NOT NULL,
    b_ref INT NOT NULL
);

-- Tabela de histórico com ENUM (Restrição de valores)
CREATE TABLE historico_validacao (
    id INT AUTO_INCREMENT PRIMARY KEY,
    modelo VARCHAR(100) NOT NULL,
    numero_serie VARCHAR(100) NOT NULL,
    status ENUM('PASS', 'FAIL') NOT NULL, -- Aceita apenas estes dois valores 
    data_hora DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Inserção de exemplo para o seu Samsung S22
INSERT INTO configuracao_cores (modelo_celular, nome_cor, r_ref, g_ref, b_ref) 
VALUES ('S22', 'Azul', 32, 72, 137); [cite: 17]