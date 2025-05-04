<?php

?>

<!DOCTYPE html>
<html lang="en">

<head>

    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="description" content="">
    <meta name="author" content="">

    <title>test</title>

    <!-- Bootstrap Core CSS -->
    <link href="../vendor/bootstrap/css/bootstrap.min.css" rel="stylesheet">
    
	<link href="https://gitcdn.github.io/bootstrap-toggle/2.2.2/css/bootstrap-toggle.min.css" rel="stylesheet">    

    <!-- MetisMenu CSS -->
    <link href="../vendor/metisMenu/metisMenu.min.css" rel="stylesheet">

    <!-- Custom CSS -->
    <link href="../dist/css/sb-admin-2.css" rel="stylesheet">

    <!-- Morris Charts CSS -->
    <link href="../vendor/morrisjs/morris.css" rel="stylesheet">

    <!-- Custom Fonts -->
    <link href="../vendor/font-awesome/css/font-awesome.min.css" rel="stylesheet" type="text/css">

    <!-- HTML5 Shim and Respond.js IE8 support of HTML5 elements and media queries -->
    <!-- WARNING: Respond.js doesn't work if you view the page via file:// -->
    <!--[if lt IE 9]>
        <script src="https://oss.maxcdn.com/libs/html5shiv/3.7.0/html5shiv.js"></script>
        <script src="https://oss.maxcdn.com/libs/respond.js/1.4.2/respond.min.js"></script>
    <![endif]--> 

    <script src="../libs/js/jquery-3.2.1.min.js"></script>    

    <script src="https://gitcdn.github.io/bootstrap-toggle/2.2.2/js/bootstrap-toggle.min.js"></script>
    	
    <script src="../libs/js/functions.js"></script> <!-- load our javascript file -->   
    
    
</head>

<body>

<input id="waziup_status_toggle_1" type="checkbox" data-toggle="toggle">
<div id="cloud_msg">cloud_msg</div>
<div id="waziup_status_console-event-test">waziup_status_console-event-test</div>

<script>
  $(function() {
	$('#waziup_status_toggle_1').change(function() {  
    	waziup_status = $(this).prop('checked');
    	$('#waziup_status_console-event-test').html('Toggle: ' + waziup_status);
    	
    	$.get("process.php", {waziup_status: waziup_status}, function(data){		
			$('#cloud_msg').html(data);
	   		 //erase message after 5 seconds
	    		setTimeout(function() {
  				$('#cloud_msg').html("");
	    		},3000);
        });
  	});
  });  
</script>

<!--        
<script>
  $(function() {
    $('#waziup_status_toggle_1').change(function() {
      $('#console-event').html('Toggle: ' + $(this).prop('checked'));
	  $('#waziup_status_console-event-test').html($(this).prop('checked'));      
    });
  });
</script>
-->

</body>
</html>