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

<div><a href="./"><h3>Get back to the display of last capture for all video nodes</h3></a></div>
  
  <div id="div_video_node">
	<h2>Captures per video node</h2>
	
	<label> Video node </label>
	<select id="select_video_node">
  		<option value="" selected></option>
  		<?php	
			for ($i = 2; $i <= 255; $i++){
				$dir = $main_dir.''.$i; // example node_1 directory for storing images from video node 1
	
				if(is_dir($dir))
					echo '<option value="'.$i.'">Node '.$i.'</option>';
			}	
		?>
	</select>
	
	<div id="div_time_interval">
	   </br>
	   <label>Time interval </label>
		 <select id="select_time_interval">
  			
		 </select>
	</div>
	
	<div id="div_pic_by_node"></div>
  </div>


</body>
</html>
