<?php
session_start();
$login_error = "";

if ($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_POST['username']) && isset($_POST['password'])) {
    $username = $_POST['username'];
    $password = $_POST['password'];

    $validPasswordHash = '$2y$12$k9QpSbIl.n05eIpnjM5IYuvRQbMYpaU9GyENxfisnqr.r6oLKW6NG';
    if ($username === 'admin' && password_verify($password, $validPasswordHash)) {
        $_SESSION['loggedin'] = true;
        header("Location: webpage.php");
        exit;
    } else {
        $login_error = "Usuário ou senha inválidos!";
    }
}
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SIGNALING</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-QWTKZyjpPEjISv5WaRU9OFeRpok6YctnYmDr5pNlyT2bRjXh0JMhjY6hW+ALEwIH" crossorigin="anonymous">
    <link href="https://fonts.googleapis.com/css2?family=Sansation&display=swap" rel="stylesheet">
    <link rel="stylesheet" href="assets/dist/login.min.css">
    <link rel="icon" href="favicon.ico" type="image/x-icon">
</head>
<body class="d-flex justify-content-center align-items-center vh-100 bg-light">
    <div class="sticky-md-top"> <img src="assets/images/logo.png" class="rounded mx-auto d-block" alt="Logo"> </div>

    <form id="loginForm" action="" method="post">
        <div class="mb-3">
            <input type="text" class="form-control" id="User" name="username" placeholder="Usuário" required>
        </div>
        <div class="mb-3">
            <input type="password" class="form-control" id="exampleInputPassword1" name="password" placeholder="Senha" required>
        </div>
        <div class="d-flex justify-content-center">
            <button type="submit" class="btn btn-primary btn-submit">Entrar</button>
        </div>
        <?php if (!empty($login_error)): ?>
            <div class="error-message"><?php echo $login_error; ?></div>
        <?php endif; ?>
    </form>

    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js" integrity="sha384-YvpcrYf0tY3lHB60NNkmXc5s9fDVZLESaAA55NDzOxhy9GkcIdslK1eN7N6jIeHz" crossorigin="anonymous"></script>
</body>
</html>
