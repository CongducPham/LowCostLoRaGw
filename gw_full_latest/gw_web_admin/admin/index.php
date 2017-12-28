<?php

// begin our session
session_start();

// check if the user is logged out
if(!isset($_SESSION['username'])){
		header('Location: pages/login.php');
		exit();
}
else{
	header('Location: pages/gateway_config.php');
}
?>
