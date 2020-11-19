<!DOCTYPE html>
<html lang="en">
	<head>
	  <title>Raspberry MongoDB</title>
	  <meta charset="utf-8">
	  <meta name="viewport" content="width=device-width, initial-scale=1">

		<!-- Bootstrap Core CSS -->
		<link href="admin/vendor/bootstrap/css/bootstrap.min.css" rel="stylesheet">
		<!-- jQuery -->
		<script src="admin/vendor/jquery/jquery.min.js"></script>

		<!-- Bootstrap Core JavaScript -->
		<script src="admin/vendor/bootstrap/js/bootstrap.min.js"></script>
		<style>
			/* Set height of the grid so .sidenav can be 100% (adjust if needed) */
			.row.content {height: 1500px}

			#banner
			{
			   background-color: #337AB7;
			   border-color: #2E6DA4;
			   background-image: none;
			   border: 1px #101010 solid;
			   -moz-border-radius: 4px;
			   -webkit-border-radius: 4px;
			   border-radius: 4px;
			}

			}
		</style>
	</head>
	<body>
		<div class="container">
			<div class="row content">
				<div class="col-sm-12">
					<div id="banner" style="text-align:center;">
						<h1 style="color:white;">Access to the data from MongoDB</h1>
						<div class="btn-group">
							<a href="./list.php" class="btn btn-default"> List</a>	
							<a href="./export_csv.php" class="btn btn-default"> Export data to csv</a>
							<a href="/admin/index.php" class="btn btn-default"> Web admin interface</a>
							
						</div>
					</div>

					<div id="flot" >
						<?php include 'flot.php';?>
					</div>
				</div>
			</div>
		</div>

	</body>
</html>












