<?php
include_once '../libs/php/functions.php';

// begin our session
session_start();

$max_attempts = 3;

$message = ""; 

$json_data = read_database_json();
$username = $json_data['username'];
$password = "";

// if default settings (i.e. username = admin and password = loragateway), encrypt password
if(check_login($json_data['username'], $json_data['password'], "admin", "loragateway")){
	$password = md5($json_data['password']);
}
else{ // password is already encrypted
	$password = $json_data['password'];
}

// create a connexion attempt session
if(!isset($_SESSION['attempt'])) $_SESSION['attempt'] = 0;

// check if the user is already logged in
if((isset($_SESSION['username'])) && (isset($_SESSION['password']))){
	//$message = 'User is already logged in';
	if(check_login($username, $password, $_SESSION['username'], $_SESSION['password'])){
		header('Location: gateway_config.php');
		exit();
	}
}

// check that both the username, password have been submitted
if(!isset( $_POST['username'], $_POST['password'])){
    //$message = 'Please enter a valid username and password!';
}
else{ // isset($_POST['username']) && isset($_POST['password'])
	
	if(!empty($_POST['username']) && !empty($_POST['password'])){

		if(check_login($username, $password, $_POST['username'], md5($_POST['password']))){
			$_SESSION['username'] = $_POST['username'];
			$_SESSION['password'] = md5($_POST['password']);
 
			header('Location: gateway_config.php');
			exit();
		}
		else{
			$_SESSION['attempt']++;
 
			header('Location: login.php');
			//$message = 'Please enter a valid username and password!';
			exit();
 
			//!\ Cette redirection est nécessaire /!\
		}
  	}
}

?>

<!DOCTYPE html>
<html lang="en">

<head>

    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="description" content="">
    <meta name="author" content="">

    <title>Login</title>

    <!-- Bootstrap Core CSS -->
    <link href="../vendor/bootstrap/css/bootstrap.min.css" rel="stylesheet">

    <!-- MetisMenu CSS -->
    <link href="../vendor/metisMenu/metisMenu.min.css" rel="stylesheet">

    <!-- Custom CSS -->
    <link href="../dist/css/sb-admin-2.css" rel="stylesheet">

    <!-- Custom Fonts -->
    <link href="../vendor/font-awesome/css/font-awesome.min.css" rel="stylesheet" type="text/css">

    <!-- HTML5 Shim and Respond.js IE8 support of HTML5 elements and media queries -->
    <!-- WARNING: Respond.js doesn't work if you view the page via file:// -->
    <!--[if lt IE 9]>
        <script src="https://oss.maxcdn.com/libs/html5shiv/3.7.0/html5shiv.js"></script>
        <script src="https://oss.maxcdn.com/libs/respond.js/1.4.2/respond.min.js"></script>
    <![endif]-->

</head>

<body>

    <div class="container">
        <div class="row">
            <div class="col-md-4 col-md-offset-4">
                <div class="login-panel panel panel-default">
                    <div class="panel-heading">
                        <h3 class="panel-title">Please Sign In</h3>
                    </div>
                    <div class="panel-body">
                        <form id="login_form" role="form" method="post" action="login.php">
                            <fieldset>
                                <div class="form-group">
                                    <input class="form-control" placeholder="Username" name="username" type="text" value="" maxlength="20" autofocus>
                                </div>
                                <div class="form-group">
                                    <input class="form-control" placeholder="Password" name="password" type="password" maxlength="20" value="">
                                </div>
                               
                                	<button  type="submit" class="btn btn-lg btn-success btn-block">Login</button>
                            </fieldset>
                        </form>
                    </div>
                </div>
                <div id="login_form_msg">
                
                </div>
            </div>
        </div>
    </div>

    <!-- jQuery -->
    <script src="../vendor/jquery/jquery.min.js"></script>

    <!-- Bootstrap Core JavaScript -->
    <script src="../vendor/bootstrap/js/bootstrap.min.js"></script>

    <!-- Metis Menu Plugin JavaScript -->
    <script src="../vendor/metisMenu/metisMenu.min.js"></script>

    <!-- Custom Theme JavaScript -->
    <script src="../dist/js/sb-admin-2.js"></script>

</body>

</html>
