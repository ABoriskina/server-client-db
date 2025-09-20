CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    login VARCHAR(50) UNIQUE NOT NULL,
    pass VARCHAR(255) NOT NULL,
    name VARCHAR(100) NOT NULL
);

INSERT INTO users (login, pass, name) VALUES
('admin', 'admin123', 'Administrator'),
('user1', 'password1', 'User One'),
('user2', 'password2', 'User Two')
