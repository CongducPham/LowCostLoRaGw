<?php
	include_once 'libs/php/functions.php';
	$main_dir = 'uploads/node_'; // main directory
	$extensions = array('bmp'); // picture extensions array: add or remove extensions
?>


<!DOCTYPE html>
<html lang="en">

<head>

    <meta charset="utf-8">

    <title>Video Monitoring</title>

    <!-- HTML5 Shim and Respond.js IE8 support of HTML5 elements and media queries -->
    <!-- WARNING: Respond.js doesn't work if you view the page via file:// -->
    <!--[if lt IE 9]>
        <script src="https://oss.maxcdn.com/libs/html5shiv/3.7.0/html5shiv.js"></script>
        <script src="https://oss.maxcdn.com/libs/respond.js/1.4.2/respond.min.js"></script>
    <![endif]-->
     <!-- jQuery -->
    <script src="libs/js/jquery.min.js"></script>

    <!-- Bootstrap Core JavaScript -->
    <script src="libs/js/bootstrap.min.js"></script>
    <script src="libs/js/functions.js"></script> <!-- load our javascript file -->

</head>

<body>

<h2><a href="./index_bak.php">Display content of image/bak folder</a></h2>
<br>

  <div id="div_last_capture">
  	<h2>Last capture for all video nodes</h2>
  	<?php
		display_last_capture_for_all_nodes($main_dir, $extensions); 
	?>
  </div>
  <div><a href="video_node.php"><h3>Captures per video node</h3></a></div>

<h3><a id="a_mv_img" href="">Move images from uploads to bak folder</a></h3>
	<div id="mv_img_msg"></div>

<br>
  
</body>
</html>
