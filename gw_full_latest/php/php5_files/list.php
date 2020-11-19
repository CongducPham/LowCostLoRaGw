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
    
  </style>

<?php
	//////////////////////////////////////
	// THIS IS TO RECOVER NOMENCLATURES //
	//////////////////////////////////////
	
	// connection with local MongoDB database
	$connection = new MongoClient();

	// select the collection from the database
	$collection = $connection->messages->ReceivedData;
	
	// recover the whole collection ascending by date
	$data = $collection->find();
	
	// array containing the nomenclatures
	$data_nom = array();
	
	while($data->hasNext()) { 
		$document = $data->getNext();
		
		// recovering json object from "data"
		$data_json = $document["data"];
		
		// saving it into an array
		$data_array = json_decode($data_json, true);
	
		foreach ($data_array as $key => $value) {
			if(count($data_nom) == 0 || !(in_array($key, $data_nom))){
				array_push($data_nom, $key);
			}
		}
	}
	
	// functions to know if a string begins and ends with another string respectively
	function startsWith($string, $part_of_string) {
	    	// search backwards starting from string length characters from the end
	    	return $part_of_string === "" || strrpos($string, $part_of_string, -strlen($string)) !== false;

	}
	function endsWith($string, $part_of_string) {
	    	// search forward starting from end minus part_of_string length characters
	    	return $part_of_string === "" || (($temp = strlen($string) - strlen($part_of_string)) >= 0 && strpos($string, $part_of_string, $temp) !== false);
	}
	
	// function to recover the end of a string after the beginning part_of_string
	function recoverEnd($string, $part_of_string) {
		return substr($string, strlen($part_of_string));
	}
	
	// functions to sort any data ascending
	function compare_data_tc($a, $b) {
		if ($a["data"]["tc"]==$b["data"]["tc"]) return 0;
		return ($a["data"]["tc"]<$b["data"]["tc"])?-1:1;
	}
	function compare_data_th($a, $b) {
		if ($a["data"]["th"]==$b["data"]["th"]) return 0;
		return ($a["data"]["th"]<$b["data"]["th"])?-1:1;
	}
	function compare_data_hu($a, $b) {
		if ($a["data"]["hu"]==$b["data"]["hu"]) return 0;

		return ($a["data"]["hu"]<$b["data"]["hu"])?-1:1;
	}
	function compare_data_lw($a, $b) {
		if ($a["data"]["lw"]==$b["data"]["lw"]) return 0;
		return ($a["data"]["lw"]<$b["data"]["lw"])?-1:1;
	}
	// functions to sort any data descending
	function reverse_compare_data_tc($a, $b) {
		if ($b["data"]["tc"]==$a["data"]["tc"]) return 0;
		return ($b["data"]["tc"]<$a["data"]["tc"])?-1:1;
	}
	function reverse_compare_data_th($a, $b) {
		if ($b["data"]["th"]==$a["data"]["th"]) return 0;
		return ($b["data"]["th"]<$a["data"]["th"])?-1:1;
	}
	function reverse_compare_data_hu($a, $b) {
		if ($b["data"]["hu"]==$a["data"]["hu"]) return 0;
		return ($b["data"]["hu"]<$a["data"]["hu"])?-1:1;

	}
	function reverse_compare_data_lw($a, $b) {
		if ($b["data"]["lw"]==$a["data"]["lw"]) return 0;
		return ($b["data"]["lw"]<$a["data"]["lw"])?-1:1;
	}
?>
<script>
$(function()
{
   $("table").each(function (){
      if($(this).attr("id") == "SlideMenu1_MoreInfo"){
         $(this).hide();
      }
   });
   
   $(".list-group-item a").click(function()
   {
      if ($(this).parent().find('table').is(':hidden'))
      {
         $(this).parent().find('table').show();
      }
      else
      {
         $(this).parent().find('table').hide();
      }
   });
});
</script>
</head>
<body>
<div class="container">
  <div class="row content">
    <div class="col-sm-12">
	<div id="banner" style="text-align:center;">
		<h1 style="color:white;">Access to the data from MongoDB</h1>
		<div class="btn-group">
			<a href="./index.php" class="btn btn-default"> Graph</a>
			<a href="./export_csv.php" class="btn btn-default"> Export data to csv</a>
			<a href="/admin/index.php" class="btn btn-default"> Web admin interface</a>

		</div>
	</div>
	<hr>
	<form name="Form1" method="GET" action="list.php" id="Form1"  style="text-align:center;" >
		<label for="Editbox1" id="Label1" >Display the</label>
		<input type="text" id="Editbox1" name="Editbox1"style="width:5%;"  value="<?php if (isset($_GET['Editbox1'])){echo $_GET['Editbox1'];}else{echo '10';} ?>">
		<select name="Combobox1" size="1" id="Combobox1" >
			<option value="first" <?php if (isset($_GET['Combobox1']) && $_GET['Combobox1'] == "first"){echo 'selected';} ?> >first</option>
			<option value="last" <?php if (isset($_GET['Combobox1']) && $_GET['Combobox1'] == "last"){echo 'selected';} ?> >last</option>
		</select>
		<label for="" id="Label2" >document(s)</label>
		<label for="" id="Label3" >and sort by</label>
		<select name="Combobox2" size="1" id="Combobox2">
			<option value="date" <?php if (isset($_GET['Combobox2']) && $_GET['Combobox2'] == "date"){echo 'selected';} ?> >date</option>
		</select>
		<input type="submit"  class="btn btn-success" id="Button1" name='Submit' value='Valid'>
	</form>
	<hr>

	<script>
		///////////////////////////
		/// filling "Combobox2" ///
		///////////////////////////
		var old_content="";
		<?php
			$value_selected = "";
			if(isset($_GET['Combobox2'])){
				$value_selected = $_GET['Combobox2'];
			} 
		?>
		<?php foreach ($data_nom as $value) { ?>
			old_content = document.getElementById('Combobox2').innerHTML;
			var content="";
			<?php if (isset($_GET['Combobox2']) && startsWith((string)$value_selected, "data_") && endsWith((string)$value_selected, (string)$value)){ ?>
				content = "<option value=\"data_"+<?php echo ("\"$value\""); ?>+"\" selected >data_"+<?php echo ("\"$value\""); ?>+"</option>";
			<?php }else{ ?>
				content = "<option value=\"data_"+<?php echo ("\"$value\""); ?>+"\" >data_"+<?php echo ("\"$value\""); ?>+"</option>";
			<?php } ?>
		
			document.getElementById('Combobox2').innerHTML = old_content + content;
		<?php } ?>
	</script>




	<?php if (isset($_GET['Submit']) && $_GET['Submit']=='Valid') { ?>
			<div id="SlideMenu1" >
			<ul class="list-group">
			<?php if (isset($_GET['Editbox1']) && isset($_GET['Combobox1'])) {
				if($_GET['Combobox1'] == "first") {
					if($_GET['Combobox2'] == "date") {
						$cursor = $collection->find();
						// sort by date ascending
						$cursor->sort(array('time' => 1));
						// number of documents traveled
						$iteration = 0;
						while($cursor->hasNext() && $iteration<intval($_GET['Editbox1'])) { 
							$document = $cursor->getNext(); ?>
							<li class="list-group-item" style="text-align:center;background-color: #337AB7;"><a class="list-group-item" ><?php echo gmdate('Y-m-d H:i:s', $document["time"]->sec); ?></a>
								<div table-responsive>
									<table class="table" id="SlideMenu1_MoreInfo" >
										<thead>
												<tr>
													<th>DATA</th>
													<th>NODE</th>
													<th>DATARATE</th>
												</tr>										
											</thead>
											<tblody>
												<tr class="success">
													<td><?php	$dataarray = json_decode($document["data"], true);
														echo "[  ";
														foreach($dataarray as $currentnom => $currentdata){
															echo "\"".strtoupper((string)$currentnom)."\"".":".$currentdata."  ";

														}
														echo "]";
												  ?></td>
													<td><?php echo $document["node_eui"]; ?></td>
													<td><?php echo $document["datarate"]; ?> CR: <?php echo $document["cr"]; ?></td>
												</tr>
											</tbody>
									</table>
								</div>
						   	</li>
							<?php $iteration += 1;
						}
					}else if(startsWith((string)$_GET['Combobox2'], "data_")){
				
						// recovering the nomenclature of the data
						$nomenclature = recoverEnd((string)$_GET['Combobox2'], "data_");
				
						$cursor = $collection->find();
				
						// auxiliary array which will contain data from MongoDB
						$aux_array = array();
			
						while($cursor->hasNext()) { 
							$document = $cursor->getNext();
				
							// to save in aux_array
							$aux_document = $document;
							// recovering the data in an array
							$aux_document["data"] = json_decode($document["data"], true);
							array_push($aux_array, $aux_document);
						}
						// sorting by the corresponding data
						usort($aux_array, "compare_data_".(string)$nomenclature);
						// number of documents traveled
						$iteration = 0;
				
						foreach($aux_array as $value){ 
							if ($iteration<intval($_GET['Editbox1'])) { ?>
								<li class="list-group-item"  style="text-align:center;background-color: #337AB7;" ><a class="list-group-item"><?php echo strtoupper((string)$nomenclature).": ".($value["data"][(string)$nomenclature]); ?></a>
									<div table-responsive>
										<table class="table" id="SlideMenu1_MoreInfo">
											<thead>
												<tr>
													<th>DATA</th>
													<th>DATE</th>
													<th>NODE</th>
													<th>DATARATE</th>
												</tr>										
											</thead>
											<tbody>
												<tr class="success">
													<td><?php echo "[  ";
															foreach($value["data"] as $currentnom => $currentdata){
																if($currentnom != $nomenclature){
																	echo "\"".strtoupper((string)$currentnom)."\"".":".$currentdata."  ";
																}
															}
															echo "]";
													  ?>
													</td>
													<td><?php echo gmdate('Y-m-d H:i:s', $value["time"]->sec); ?></td>
													<td><?php echo $value["node_eui"]; ?></td>
													<td><?php echo $value["datarate"]; ?> CR: <?php echo $value["cr"]; ?></td>
												</tr>
											</tbody>
										</table>
									</div>
							   	</li>
						<?php		$iteration += 1;
							}
						}
					}
				}else{
					if($_GET['Combobox2'] == "date") {
		
						$cursor = $collection->find();
						// sort by date ascending
						$cursor->sort(array('time' => -1));
						// number of documents traveled
						$iteration = 0;
			
						while($cursor->hasNext() &&  $iteration<intval($_GET['Editbox1'])) { 
						$document = $cursor->getNext(); ?>
						<li class="list-group-item"  style="text-align:center;background-color: #337AB7;"><a class="list-group-item"><?php echo gmdate('Y-m-d H:i:s', $document["time"]->sec); ?></a>
							<div table-responsive>
								<table class="table" id="SlideMenu1_MoreInfo">
									<thead>
										<tr>
											<th>DATA</th>
											<th>NODE</th>
											<th>DATARATE</th>
										</tr>										
									</thead>
									<tbody>
										<tr  class="success">
											<td><?php	$dataarray = json_decode($document["data"], true);
													echo "[  ";
													foreach($dataarray as $currentnom => $currentdata){
														echo "\"".strtoupper((string)$currentnom)."\"".":".$currentdata."  ";
													}
													echo "]";
											  ?></td>
											<td><?php echo $document["node_eui"]; ?></td>
											<td><?php echo $document["datarate"]; ?> CR: <?php echo $document["cr"]; ?></td>
										</tr>
									</tbody>
									</table>
							</div>
					   	</li>
						<?php $iteration += 1;
						}
					}else if(startsWith((string)$_GET['Combobox2'], "data_")){
				
						// recovering the nomenclature of the data
						$nomenclature = recoverEnd((string)$_GET['Combobox2'], "data_");
				
						$cursor = $collection->find();
						// sort by date descending
						$cursor->sort(array('time' => -1));
						// number of documents traveled
						$iteration = 0;
				
						// auxiliary array which will contain data from MongoDB
						$aux_array = array();
			
						while($cursor->hasNext()) { 
							$document = $cursor->getNext();
				
							// to save in aux_array
							$aux_document = $document;
							// recovering the data in an array
							$aux_document["data"] = json_decode($document["data"], true);
							array_push($aux_array, $aux_document);
						}
						// sorting by the corresponding data
						usort($aux_array, "reverse_compare_data_".(string)$nomenclature);
						// number of documents traveled
						$iteration = 0;
				
						foreach($aux_array as $value){ 
							if ($iteration<intval($_GET['Editbox1'])) { ?>

								<li class="list-group-item"  style="text-align:center;background-color: #337AB7;"><a class="list-group-item"><?php echo strtoupper((string)$nomenclature).": ".($value["data"][(string)$nomenclature]); ?></a>
									<div table-responsive>
										<table class="table" id="SlideMenu1_MoreInfo">
											<thead>
												<tr>
													<th>DATA</th>
													<th>DATE</th>
													<th>NODE</th>
													<th>DATARATE</th>
												</tr>										
											</thead>
											<tbody>
												<tr  class="success">
													<td>
													 <?php echo "[  ";
																foreach($value["data"] as $currentnom => $currentdata){
																	if($currentnom != $nomenclature){
																		echo "\"".strtoupper((string)$currentnom)."\"".":".$currentdata."  ";
																	}
																}
																echo "]";
														  ?>
													</td>
													<td><?php echo gmdate('Y-m-d H:i:s', $value["time"]->sec); ?></td>
													<td><?php echo $value["node_eui"]; ?></td>
													<td><?php echo $value["datarate"]; ?> CR: <?php echo $value["cr"]; ?></td>
												</tr>

											</tbody>
								      		</table>
									</div>
							   	</li>
						<?php		$iteration += 1;
							}
						}
					}
				}
			} ?>
			</ul>
			</div>
		<?php } ?>

		</div>
	</div>
</div>
</body>

    </div>
