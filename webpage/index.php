<?php
session_start();

require_once __DIR__ . '/classes/UserRepository.php';
require_once __DIR__ . '/classes/Countries.php';

$login_error    = '';
$register_error = '';
$register_data  = ['name' => '', 'email' => '', 'country' => ''];
$active_form    = 'login';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {

    // ===== Cadastro =====
    if (isset($_POST['action']) && $_POST['action'] === 'register') {
        $active_form = 'register';

        $name             = trim($_POST['name'] ?? '');
        $email            = strtolower(trim($_POST['email'] ?? ''));
        $country          = strtoupper(trim($_POST['country'] ?? ''));
        $password         = $_POST['password'] ?? '';
        $password_confirm = $_POST['password_confirm'] ?? '';

        $register_data = ['name' => $name, 'email' => $email, 'country' => $country];

        try {
            if ($name === '' || $email === '' || $country === '' || $password === '') {
                throw new RuntimeException('Preencha todos os campos.');
            }
            if (!filter_var($email, FILTER_VALIDATE_EMAIL)) {
                throw new RuntimeException('E-mail inválido.');
            }
            if (!Countries::isValidCode($country)) {
                throw new RuntimeException('País inválido.');
            }
            if (strlen($password) < 8) {
                throw new RuntimeException('Senha deve ter no mínimo 8 caracteres.');
            }
            if ($password !== $password_confirm) {
                throw new RuntimeException('As senhas não conferem.');
            }
            if (UserRepository::emailExists($email)) {
                throw new RuntimeException('Já existe uma conta com esse e-mail.');
            }

            $user = UserRepository::create($name, $email, $country, $password);

            // Auto-login
            $_SESSION['loggedin'] = true;
            $_SESSION['user_id']  = $user['id'];
            $_SESSION['user_email'] = $user['email'];
            header('Location: webpage.php');
            exit;

        } catch (RuntimeException $e) {
            $register_error = $e->getMessage();
        } catch (Throwable $e) {
            $register_error = 'Erro ao criar conta. Tente novamente.';
        }
    }

    // ===== Login =====
    elseif (isset($_POST['email']) && isset($_POST['password'])) {
        $email    = strtolower(trim($_POST['email']));
        $password = $_POST['password'];

        try {
            $user = UserRepository::findByEmail($email);
            if ($user && UserRepository::verifyPassword($user, $password)) {
                $_SESSION['loggedin']   = true;
                $_SESSION['user_id']    = $user['id'];
                $_SESSION['user_email'] = $user['email'];
                header('Location: webpage.php');
                exit;
            }
            $login_error = 'E-mail ou senha inválidos!';
        } catch (Throwable $e) {
            $login_error = 'Erro de autenticação. Tente novamente.';
        }
    }
}

$countries = Countries::all();
?>

<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ATSC 3.0 SIGNALING - Calculadora de Sinalização L1</title>
    <link rel="stylesheet" href="assets/dist/login.min.css">
    <link rel="icon" href="favicon.ico" type="image/x-icon">
</head>
<body>
    <header class="top-bar">
        <div class="top-bar-inner">
            <div class="brand">
                <img src="assets/images/logo.png" alt="Logo" class="brand-logo">
                <span class="brand-text">ATSC<br>SIGNALING</span>
            </div>

            <form id="loginForm" class="login-inline" action="" method="post">
                <div class="login-fields">
                    <label class="login-field">
                        <span>E-mail</span>
                        <input type="email" name="email" required autocomplete="username">
                    </label>
                    <label class="login-field">
                        <span>Senha</span>
                        <input type="password" name="password" required autocomplete="current-password">
                    </label>
                </div>
                <button type="submit" class="btn-login">Entrar</button>
            </form>
        </div>
        <?php if (!empty($login_error)): ?>
            <div class="login-error-bar"><?php echo htmlspecialchars($login_error); ?></div>
        <?php endif; ?>
    </header>

    <main class="welcome">
        <section class="hero">
            <div class="hero-content">
                <h1>Calculadora de Sinalização ATSC 3.0</h1>
                <p class="hero-subtitle">
                    Ferramenta de cálculo e configuração da camada física do padrão de TV digital de próxima geração.
                </p>
                <p class="hero-description">
                    Configure os parâmetros de <strong>Bootstrap</strong>, <strong>Preâmbulo</strong>, <strong>Subframes</strong> e <strong>PLPs</strong>
                    de acordo com o padrão ATSC 3.0 (A/321 e A/322), com cálculos automáticos de duração de frame, taxa de bits e capacidade do canal.
                </p>
                <a href="#sobre" class="hero-cta">Saiba mais</a>
            </div>

            <aside class="register-card" id="cadastro">
                <h2 class="register-title">Criar conta</h2>
                <p class="register-subtitle">Cadastre-se para começar a usar a ferramenta.</p>

                <?php if (!empty($register_error)): ?>
                    <div class="register-error" role="alert">
                        <?php echo htmlspecialchars($register_error); ?>
                    </div>
                <?php endif; ?>

                <form action="" method="post" class="register-form" novalidate>
                    <input type="hidden" name="action" value="register">

                    <label class="register-field">
                        <span>Nome</span>
                        <input type="text" name="name" maxlength="120" required
                               value="<?php echo htmlspecialchars($register_data['name']); ?>"
                               autocomplete="name">
                    </label>

                    <label class="register-field">
                        <span>E-mail</span>
                        <input type="email" name="email" maxlength="255" required
                               value="<?php echo htmlspecialchars($register_data['email']); ?>"
                               autocomplete="email">
                    </label>

                    <label class="register-field">
                        <span>País</span>
                        <select name="country" required>
                            <option value="">Selecione...</option>
                            <?php foreach ($countries as $code => $label): ?>
                                <option value="<?php echo $code; ?>"
                                    <?php echo $register_data['country'] === $code ? 'selected' : ''; ?>>
                                    <?php echo htmlspecialchars($label); ?>
                                </option>
                            <?php endforeach; ?>
                        </select>
                    </label>

                    <div class="register-row">
                        <label class="register-field">
                            <span>Senha</span>
                            <input type="password" name="password" minlength="8" required
                                   autocomplete="new-password">
                        </label>
                        <label class="register-field">
                            <span>Confirmar senha</span>
                            <input type="password" name="password_confirm" minlength="8" required
                                   autocomplete="new-password">
                        </label>
                    </div>

                    <button type="submit" class="btn-register">Cadastrar</button>
                </form>
            </aside>
        </section>

        <section class="frame-section">
            <div class="hero-visual" aria-hidden="true">
                <div class="frame-diagram">
                    <div class="frame-block bootstrap">
                        <span class="frame-block-label">Bootstrap</span>
                        <span class="frame-block-sub">~2 ms</span>
                    </div>
                    <div class="frame-block preamble">
                        <span class="frame-block-label">Preâmbulo</span>
                        <span class="frame-block-sub">L1-Basic / L1-Detail</span>
                    </div>
                    <div class="frame-block payload">
                        <span class="frame-block-label">Subframes &amp; PLPs</span>
                        <span class="frame-block-sub">payload OFDM</span>
                    </div>
                </div>
                <div class="frame-axis">
                    <span>0</span>
                    <span>tempo &rarr;</span>
                    <span>frame</span>
                </div>
            </div>
        </section>

        <section id="sobre" class="features">
            <h2>O que a ferramenta faz</h2>
            <div class="features-grid">
                <article class="feature-card">
                    <div class="feature-icon" aria-hidden="true">
                        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                            <path d="M3 12h4l3-9 4 18 3-9h4"/>
                        </svg>
                    </div>
                    <h3>Bootstrap</h3>
                    <p>Define versão major/minor, símbolos, EA wakeup, largura de banda e tempo até o próximo frame conforme A/321.</p>
                </article>

                <article class="feature-card">
                    <div class="feature-icon" aria-hidden="true">
                        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                            <rect x="3" y="4" width="18" height="16" rx="2"/>
                            <path d="M3 10h18"/>
                            <path d="M9 4v16"/>
                        </svg>
                    </div>
                    <h3>Preâmbulo L1</h3>
                    <p>Configura L1-Basic e L1-Detail: FEC, modulação, time info, frame length mode, BSID e versão de sinalização.</p>
                </article>

                <article class="feature-card">
                    <div class="feature-icon" aria-hidden="true">
                        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                            <rect x="3" y="6" width="6" height="12" rx="1"/>
                            <rect x="11" y="6" width="6" height="12" rx="1"/>
                            <rect x="19" y="6" width="2" height="12" rx="1"/>
                        </svg>
                    </div>
                    <h3>Subframes &amp; PLPs</h3>
                    <p>Define múltiplos subframes com diferentes estruturas OFDM e Physical Layer Pipes para multiplexação de serviços.</p>
                </article>

                <article class="feature-card">
                    <div class="feature-icon" aria-hidden="true">
                        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                            <path d="M12 2v4"/>
                            <path d="M12 18v4"/>
                            <path d="M4.93 4.93l2.83 2.83"/>
                            <path d="M16.24 16.24l2.83 2.83"/>
                            <path d="M2 12h4"/>
                            <path d="M18 12h4"/>
                            <circle cx="12" cy="12" r="4"/>
                        </svg>
                    </div>
                    <h3>Cálculos Automáticos</h3>
                    <p>Duração de frame, taxa de dados, capacidade total e validação de parâmetros calculadas em tempo real.</p>
                </article>
            </div>
        </section>

        <section class="workflow">
            <h2>Como funciona</h2>
            <div class="workflow-steps">
                <div class="workflow-step">
                    <div class="workflow-step-number">1</div>
                    <h4>Bootstrap</h4>
                    <p>Configure os parâmetros básicos do sinal e a largura de banda.</p>
                </div>
                <div class="workflow-arrow" aria-hidden="true">&rarr;</div>
                <div class="workflow-step">
                    <div class="workflow-step-number">2</div>
                    <h4>Preâmbulo</h4>
                    <p>Defina L1-Basic e L1-Detail com FEC, modulação e tempo.</p>
                </div>
                <div class="workflow-arrow" aria-hidden="true">&rarr;</div>
                <div class="workflow-step">
                    <div class="workflow-step-number">3</div>
                    <h4>Subframes</h4>
                    <p>Adicione subframes OFDM com diferentes parâmetros físicos.</p>
                </div>
                <div class="workflow-arrow" aria-hidden="true">&rarr;</div>
                <div class="workflow-step">
                    <div class="workflow-step-number">4</div>
                    <h4>PLPs</h4>
                    <p>Configure os Physical Layer Pipes e gere a sinalização.</p>
                </div>
            </div>
        </section>

        <section class="about">
            <h2>Sobre o padrão ATSC 3.0</h2>
            <p>
                O <strong>ATSC 3.0</strong> é o padrão de televisão digital terrestre de próxima geração desenvolvido pelo
                Advanced Television Systems Committee. Utiliza modulação OFDM, codificação LDPC/BCH e suporta ultra-alta
                definição (4K/8K), HDR, áudio imersivo e transmissão híbrida broadcast/broadband.
            </p>
            <p>
                Esta ferramenta auxilia engenheiros e pesquisadores a explorar combinações de parâmetros da camada física
                (PHY) e validar configurações conforme as especificações <strong>A/321</strong> (Bootstrap) e
                <strong>A/322</strong> (Physical Layer Protocol).
            </p>
        </section>

        <footer class="welcome-footer">
            <p>ATSC 3.0 Signaling Calculator &mdash; projeto de mestrado</p>
        </footer>
    </main>

    <?php if ($active_form === 'register'): ?>
    <script>
        document.getElementById('cadastro').scrollIntoView({behavior: 'smooth', block: 'start'});
    </script>
    <?php endif; ?>
</body>
</html>
