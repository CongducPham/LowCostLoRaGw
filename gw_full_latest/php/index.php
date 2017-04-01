<!--
*
* Copyright (C) 2016 Nicolas Bertuol, University of Pau, France
*
* nicolas.bertuol@etud.univ-pau.fr
*
-->

<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>Raspberry MongoDB</title>

<link rel="stylesheet" href="style.php" media="screen">

<script language="javascript" type="text/javascript" src="jquery.js"></script>

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
   $("ul").each(function (){
      if($(this).attr("id") == "SlideMenu1_MoreInfo"){
         $(this).hide();
      }
   });
   
   $(".SlideMenu1_RecupDB a").click(function()
   {
      if ($(this).parent().find('ul').is(':hidden'))
      {
         $(this).parent().find('ul').show();
      }
      else
      {
         $(this).parent().find('ul').hide();
      }
   });
});
</script>
</head>
<body>

<div id="banner" style="position:absolute;left:0%;top:0%;width:1345px;height:146px;z-index:9;">
	<h1>Access to the data from MongoDB</h1>
	<a href="./export_csv.php" style="position:absolute;color:#FFFFFF;left:1150px;top:100px;z-index:9;">export data to csv</a> 
</div>



<div id="wb_Form1" style="position:absolute;left:5px;top:154px;width:495px;height:154px;z-index:9;">
<form name="Form1" method="GET" action="index.php" id="Form1">
	<label for="Editbox1" id="Label1" style="position:absolute;left:10px;top:18px;width:69px;height:18px;line-height:18px;z-index:1;">Display the</label>
	<input type="text" id="Editbox1" style="position:absolute;left:87px;top:17px;width:25px;height:18px;line-height:18px;z-index:0;" name="Editbox1" value="<?php if (isset($_GET['Editbox1'])){echo $_GET['Editbox1'];}else{echo '10';} ?>">
	<select name="Combobox1" size="1" id="Combobox1" style="position:absolute;left:131px;top:17px;width:100px;height:28px;z-index:2;">
		<option value="first" <?php if (isset($_GET['Combobox1']) && $_GET['Combobox1'] == "first"){echo 'selected';} ?> >first</option>
		<option value="last" <?php if (isset($_GET['Combobox1']) && $_GET['Combobox1'] == "last"){echo 'selected';} ?> >last</option>
	</select>
	<label for="" id="Label2" style="position:absolute;left:238px;top:18px;width:76px;height:18px;line-height:18px;z-index:3;">document(s)</label>
	
	<label for="" id="Label3" style="position:absolute;left:10px;top:69px;width:69px;height:18px;line-height:18px;z-index:4;">Sort by</label>
	<select name="Combobox2" size="1" id="Combobox2" style="position:absolute;left:87px;top:68px;width:100px;height:28px;z-index:5;">
		<option value="date" <?php if (isset($_GET['Combobox2']) && $_GET['Combobox2'] == "date"){echo 'selected';} ?> >date</option>
	</select>
		
	<input type="submit" id="Button1" name='Submit' value='Valid' style="position:absolute;left:80%;top:80%;width:75px;height:25px;z-index:7;">
</form>
</div>	

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
	<div id="SlideMenu1" style="position:absolute;left:5px;top:323px;width:495px;height:350px;display:block;">
	<ul>
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
					<li class="SlideMenu1_RecupDB"><a><?php echo gmdate('Y-m-d H:i:s', $document["time"]->sec); ?></a>
						<ul id="SlideMenu1_MoreInfo">
							<li><p>DATA: <?php	$dataarray = json_decode($document["data"], true);
												echo "[  ";
												foreach($dataarray as $currentnom => $currentdata){
													echo "\"".strtoupper((string)$currentnom)."\"".":".$currentdata."  ";
												}
												echo "]";
										  ?>
							</p></li>
							<li><p>NODE: <?php echo $document["node_eui"]; ?></p></li>
							<li><p>DATARATE: <?php echo $document["datarate"]; ?> CR: <?php echo $document["cr"]; ?></p></li>
				      		</ul>
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
						<li class="SlideMenu1_RecupDB"><a><?php echo strtoupper((string)$nomenclature).": ".($value["data"][(string)$nomenclature]); ?></a>
							<ul id="SlideMenu1_MoreInfo">
								<li><p>DATA: <?php echo "[  ";
													foreach($value["data"] as $currentnom => $currentdata){
														if($currentnom != $nomenclature){
															echo "\"".strtoupper((string)$currentnom)."\"".":".$currentdata."  ";
														}
													}
													echo "]";
											  ?>
								</p></li>
								<li><p>DATE: <?php echo gmdate('Y-m-d H:i:s', $value["time"]->sec); ?></p></li>
								<li><p>NODE: <?php echo $value["node_eui"]; ?></p></li>
								<li><p>DATARATE: <?php echo $value["datarate"]; ?> CR: <?php echo $value["cr"]; ?></p></li>
					      		</ul>
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
				<li class="SlideMenu1_RecupDB"><a><?php echo gmdate('Y-m-d H:i:s', $document["time"]->sec); ?></a>
					<ul id="SlideMenu1_MoreInfo">
						<li><p>DATA: <?php	$dataarray = json_decode($document["data"], true);
											echo "[  ";
											foreach($dataarray as $currentnom => $currentdata){
												echo "\"".strtoupper((string)$currentnom)."\"".":".$currentdata."  ";
											}
											echo "]";
									  ?>
						</p></li>
						<li><p>NODE: <?php echo $document["node_eui"]; ?></p></li>
						<li><p>DATARATE: <?php echo $document["datarate"]; ?> CR: <?php echo $document["cr"]; ?></p></li>
			      		</ul>
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
						<li class="SlideMenu1_RecupDB"><a><?php echo strtoupper((string)$nomenclature).": ".($value["data"][(string)$nomenclature]); ?></a>
							<ul id="SlideMenu1_MoreInfo">
								<li><p>DATA: <?php echo "[  ";
													foreach($value["data"] as $currentnom => $currentdata){
														if($currentnom != $nomenclature){
															echo "\"".strtoupper((string)$currentnom)."\"".":".$currentdata."  ";
														}
													}
													echo "]";
											  ?>
								</p></li>
								<li><p>DATE: <?php echo gmdate('Y-m-d H:i:s', $value["time"]->sec); ?></p></li>
								<li><p>NODE: <?php echo $value["node_eui"]; ?></p></li>
								<li><p>DATARATE: <?php echo $value["datarate"]; ?> CR: <?php echo $value["cr"]; ?></p></li>
					      		</ul>
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



<div id="flot" style="position:absolute;left:500px;top:154px;width:790px;height:307px;">
	<?php include 'flot.php';?>
</div>

</body>
</html>
