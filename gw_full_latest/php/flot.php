<!--
*
* Copyright (C) 2016 Nicolas Bertuol, University of Pau, France
*
* nicolas.bertuol@etud.univ-pau.fr
*
-->


	<link href="flot.css" rel="stylesheet" type="text/css">
	<script language="javascript" type="text/javascript" src="jquery.js"></script>
	<script language="javascript" type="text/javascript" src="jquery.flot.js"></script>
	<script language="javascript" type="text/javascript" src="jquery.flot.time.js"></script>
	
		
	<?php 
		// connection with local MongoDB database
		$connection = new MongoClient();
	
		// select the collection from the database
		$collection = $connection->messages->ReceivedData;
	
		// recover the whole collection ascending by date
		$data = $collection->find();
		
		// sort by source and then by date
		$data->sort(array('node_eui' => 1, 'time' => 1));
	?>
	
	<script type="text/javascript">
		// return the date in millis of UTC since jan 1 1970 for the graph
		function getMillisUTC(date_millis) {
			var date = new Date();
			date.setTime(date_millis);
			
			// recover UTC time in millis
			var millis_utc = date.getTime() + (date.getTimezoneOffset() * 60000);
			
			return millis_utc;
		}
		
		// check if a source is checked or not and returns the result
		function source_is_checked(src){
			return document.getElementById(src).checked == true;
		}
		
		// add all checkboxes corresponding to the sources
		function add_checkboxes_sources(){
			for(var i=0; i<sources.length; i++){
				new_checkbox_source("node_"+sources[i], true);
			}
		}
		
		// same function as above with a parameter, but not optimized
		function add_checkboxes_check_array(check_array){
			for(var i=0; i<sources.length; i++){
				// check if checkbox was checked
				var checked = false;
				
				for(var j=0; j<check_array.length; j++){
					if (sources[i] == check_array[j]){
						checked = true;
					}
				}
				if (checked){
					new_checkbox_source("node_"+sources[i], true);
				}else{
					new_checkbox_source("node_"+sources[i], false);
				}
			}
		}
		
		// check if a data is checked or not and returns the result
		function data_is_checked(dataName){
			return document.getElementById(dataName).checked == true;
		}
		
		// add all radios corresponding to the data nomenclatures
		function add_radios_data(){
			for(var i=0; i<data_nom.length; i++){
				if(i == 0){
					new_radio_data(data_nom[i], true);
				}else{
					new_radio_data(data_nom[i], false);
				}
			}
		}
		
		// same function as above with a parameter
		function add_radios_radio_check(radio_check){
			for(var i=0; i<data_nom.length; i++){
				if(data_nom[i] == radio_check){
					new_radio_data(data_nom[i], true);
				}else{
					new_radio_data(data_nom[i], false);
				}
			}
		}
		
		// set a cookie with an expiration date (exdays is in days)
		function set_cookie(cname, cvalue, exdays) {
			var d = new Date();
			d.setTime(d.getTime() + (exdays*24*60*60*1000));
			var expires = "expires="+d.toUTCString();
			document.cookie = cname + "=" + cvalue + "; " + expires;
		}
		
		// function to check if a cookie exists or not
		function get_cookie(cname) {
			var name = cname + "=";
			var ca = document.cookie.split(';');
			for(var i=0; i<ca.length; i++) {
				var c = ca[i];
				while (c.charAt(0)==' ') c = c.substring(1);
				if (c.indexOf(name) == 0) return c.substring(name.length,c.length);
			}
			return "";
		}
		
		// check cookie content
		function check_cookie(cname_checkbox, cname_zoom, cname_radio) {
			var checkboxes = get_cookie(cname_checkbox);
			var zoom = get_cookie(cname_zoom);
			var radios = get_cookie(cname_radio);
			// checkboxes
			if (checkboxes!="") {
				// recover the checkboxes which were checked in an array
				check_array = checkboxes.split("-");
				add_checkboxes_check_array(check_array);
			}else{
				add_checkboxes_sources();
			}
			// radios
			if (radios != ""){
				// recover the radio which was checked in a variable
				add_radios_radio_check(radios);
			}else{
				add_radios_data();
			}
			
			// zoom
			if (zoom!="") {
				return zoom;
			}else{
				return "#whole";
			}
		}
	
		// add a new "<input type="checkbox">" for the sources and check it if it has to be
		function new_checkbox_source(srcName, isChecked){
			old_content = document.getElementById('sources').innerHTML;
			if (isChecked){
				content = "<input type='checkbox' id='" + srcName + "' name='checkboxes_sources' value='" + srcName + "' checked /> " + srcName;
				document.getElementById('sources').innerHTML = old_content + content;
			}else{
				content = "<input type='checkbox' id='" + srcName + "' name='checkboxes_sources' value='" + srcName + "' /> " + srcName;
				document.getElementById('sources').innerHTML = old_content + content;
			}
		}
		
		// add a new "<input type="radio">" for the data 
		function new_radio_data(dataName, isChecked){
			old_content = document.getElementById('data').innerHTML;
			if (isChecked){
				content = "<input type='radio' id='" + dataName + "' name='radio_data' value='" + dataName + "' checked /> " + dataName;
				document.getElementById('data').innerHTML = old_content + content;
			}else{
				content = "<input type='radio' id='" + dataName + "' name='radio_data' value='" + dataName + "' /> " + dataName;
				document.getElementById('data').innerHTML = old_content + content;
			}
		}
	
		var srcArray = [];
		// to be add in srcArray
		var auxSrc = [];
		// to be add in auxSrc
		var auxDataSave = {};
		// contains all sources (example 3, 6, 8...)
		var sources = [];
		// contains all data nomenclatures
		var data_nom = [];
		// to recover json object from "data"
		var data_json="";
		// add all values sorted by node_id in different arrays
		<?php while($data->hasNext()) { $document = $data->getNext(); ?>
			// first occurrence
			if(sources.length == 0){
				sources.push('<?php echo $document["node_eui"]; ?>');
			// different source
			}else if(sources[sources.length-1] != '<?php echo $document["node_eui"]; ?>'){
				srcArray.push(auxSrc);
				sources.push('<?php echo $document["node_eui"]; ?>');
				auxSrc = [];
			}
			
			// recovering json object from "data"
			data_json = jQuery.parseJSON( '<?php echo $document["data"]; ?>' );
			
			for (var key in data_json) {
				// adds key in data_nom if does not already exists, $.inArray returns -1 if false
				if(data_nom.length == 0 || $.inArray(key, data_nom) == -1){
			  		data_nom.push(key);
			  	}
			  	auxDataSave["data_"+key] = ['<?php echo ($document["time"]->sec*1000); ?>', data_json[key]];
			}
			
			auxSrc.push(auxDataSave);
			auxDataSave = {};
			
		<?php } ?>
		// save the last auxSrc array in srcArray
		srcArray.push(auxSrc);
	</script>	
	
	
	
		
	<script type="text/javascript">
	
	function make_graph(display_array){
		$.plot("#placeholder", display_array,
			{
			xaxis: { mode: "time",
				 timezone: "browser" },
			series: {
				lines: {
					show: true
				},
				points: {
					show: false
				}
			},
			grid: {
				hoverable: true,
				clickable: false
			},
			legend: {   
			    	position: "ne",            
    				noColumns: 0
			}
		});
	}

	$(function() {
		// adding values in arrays to see it in the graph
		// for each source
		for (var i=0; i<srcArray.length; i++){
			// for each object
			for(var j=0; j<srcArray[i].length; j++){
				// iterate through keys from the object
				for(var key in srcArray[i][j]){
					// key[0] = date, key[1] = data
					var aux = [getMillisUTC((srcArray[i][j])[key][0]), parseFloat((srcArray[i][j])[key][1])];
					(srcArray[i][j])[key] = aux;
				}
			}
		}
		
		// recover cookies (if exists)
		var current_zoom = check_cookie("checkboxes", "zoom", "radio");
		
		// update zoom
		$(current_zoom).trigger( "click" );
		
		// contains sources values to display on the graph
		var display_array = [];
		
		// filling "display_array"
		for (var i=0; i<sources.length; i++){
			if (source_is_checked("node_"+sources[i])){
				for(var j=0; j<data_nom.length; j++){
					if(data_is_checked(data_nom[j])){
						// filling a new array corresponding to the data selected and to the source
						var array_to_fill = [];
						// for each object of the current source, recover the right data
						for(var iteration=0; iteration<srcArray[i].length; iteration++){
							array_to_fill.push((srcArray[i][iteration])["data_"+data_nom[j]]);
						}
						display_array.push(
							{ data: array_to_fill, label: "node "+sources[i]}
						);
					}
				}
			}
		}
		
		// if checkboxes are modified, recall make_graph() with correct sources
		$("#sources").change(function(){
			$(this).change_checkboxes();
		});

		$.fn.change_checkboxes = function(){ 
	
			display_array = [];
			var checkboxes = "";
			
			// filling "display_array"
			for (var i=0; i<sources.length; i++){
				if (source_is_checked("node_"+sources[i])){
					for(var j=0; j<data_nom.length; j++){
						if(data_is_checked(data_nom[j])){
							// filling a new array corresponding to the data selected and to the source
							var array_to_fill = [];
							// for each object of the current source, recover the right data
							for(var iteration=0; iteration<srcArray[i].length; iteration++){
								array_to_fill.push((srcArray[i][iteration])["data_"+data_nom[j]]);
							}
							display_array.push(
								{ data: array_to_fill, label: "node "+sources[i]}
							);
						}
					}
					// completing the array which will set the cookie "checkboxes"
					if(checkboxes == ""){
						checkboxes += sources[i];
					}else{
						checkboxes += "-"+sources[i];
					}
				}
			}
			
			// set a new cookie with checkboxes
			set_cookie("checkboxes", checkboxes, 1);
			
			// simulate click to be at the same zoom than before checking a checkbox
			$( current_zoom ).trigger( "click" );
		};
		
		// if radios are modified, recall make_graph() with correct data
		$("#data").change(function(){
			$(this).change_radios();
		});

		$.fn.change_radios = function(){ 
	
			display_array = [];
			var radio = "";
		
			for (var i=0; i<sources.length; i++){
				if (source_is_checked("node_"+sources[i])){
					for(var j=0; j<data_nom.length; j++){
						if(data_is_checked(data_nom[j])){
							// filling a new array corresponding to the data selected and to the source
							var array_to_fill = [];
							// for each object of the current source, recover the right data
							for(var iteration=0; iteration<srcArray[i].length; iteration++){
								array_to_fill.push((srcArray[i][iteration])["data_"+data_nom[j]]);
							}
							display_array.push(
								{ data: array_to_fill, label: "node "+sources[i]}
							);
							// setting the variable which will set the cookie "radio"
							if(radio == ""){
								radio = data_nom[j];
							}
						}
					}
				}
			}
			
			// set a new cookie with checkboxes
			set_cookie("radio", radio, 1);
			
			// simulate click to be at the same zoom than before checking a checkbox
			$( current_zoom ).trigger( "click" );
		};
		
		make_graph(display_array);
		

		$("#whole").click(function () {
			$.plot("#placeholder", display_array,
			{
				xaxis: { mode: "time",
					 timezone: "browser" },
				series: {
					lines: {
						show: true
					},
					points: {
						show: false
					}
				},
				grid: {
					hoverable: true,
					clickable: false
				},
				legend: {   
				    	position: "ne",            
            				noColumns: 0
				}
			});
			current_zoom = "#whole";
			
			// set a new cookie with zoom
			set_cookie("zoom", current_zoom, 1);
		});

		$("#lastmonth").click(function () {
			var datemax = new Date();
			
			datemax.setDate(1);
			datemax.setHours(0);
			datemax.setMinutes(0);
			datemax.setSeconds(0);
			
			var datemin = new Date(datemax.getTime());
			datemin.setMonth(datemax.getMonth()-1);
			
			$.plot("#placeholder", display_array,
			{
				xaxis: {
					mode: "time",
					timezone: "browser",
					minTickSize: [1, "day"],
					min: datemin.getTime(),
					max: datemax.getTime()
				},
				series: {
					lines: {
						show: true
					},
					points: {
						show: false
					}
				},
				grid: {
					hoverable: true,
					clickable: false
				},
				legend: {   
				    	position: "ne",            
            				noColumns: 0
				}
			});
			current_zoom = "#lastmonth";
			
			// set a new cookie with zoom
			set_cookie("zoom", current_zoom, 1);
		});
		
		$("#currentmonth").click(function () {
			var datemax = new Date();
			var datemin = new Date(datemax.getTime());
			datemin.setDate(1);
			datemin.setHours(0);
			datemin.setMinutes(0);
			datemin.setSeconds(0);
			
			$.plot("#placeholder", display_array,
			{
				xaxis: {
					mode: "time",
					timezone: "browser",
					minTickSize: [1, "day"],
					min: datemin.getTime(),
					max: datemax.getTime()
				},
				series: {
					lines: {
						show: true
					},
					points: {
						show: false
					}
				},
				grid: {
					hoverable: true,
					clickable: false
				},
				legend: {   
				    	position: "ne",            
            				noColumns: 0
				}
			});
			current_zoom = "#currentmonth";
			
			// set a new cookie with zoom
			set_cookie("zoom", current_zoom, 1);
		});

		$("#lastweek").click(function () {
			var datemax = new Date();
			var datemin = new Date(datemax.getTime());
			datemax.setHours(23);
			datemax.setMinutes(59);
			datemax.setSeconds(59);
			datemin.setHours(0);
			datemin.setMinutes(0);
			datemin.setSeconds(0);
			datemin.setDate(datemax.getDate()-6);
			
			$.plot("#placeholder", display_array,
			{
				xaxis: {
					mode: "time",
					timezone: "browser",
					minTickSize: [1, "day"],
					min: datemin.getTime(),
					max: datemax.getTime(),
					timeformat: "%a"
				},
				series: {
					lines: {
						show: true
					},
					points: {
						show: false
					}
				},
				grid: {
					hoverable: true,
					clickable: false
				},
				legend: {   
				    	position: "ne",            
            				noColumns: 0
				}
			});
			current_zoom = "#lastweek";
			
			// set a new cookie with zoom
			set_cookie("zoom", current_zoom, 1);
		});

		$("#currentday").click(function () {
			var datemin = new Date();
			var datemax = new Date(datemin.getTime());
			
			datemin.setHours(0);
			datemin.setMinutes(0);
			datemin.setSeconds(0);
			
			datemax.setHours(23);
			datemax.setMinutes(59);
			datemax.setSeconds(59);
			$.plot("#placeholder", display_array,
			{
				xaxis: {
					mode: "time",
					timezone: "browser",
					minTickSize: [1, "hour"],
					min: (datemin.getTime()),
					max: (datemax.getTime()),
					twelveHourClock: true
				},
				series: {
					lines: {
						show: true
					},
					points: {
						show: false
					}
				},
				grid: {
					hoverable: true,
					clickable: false
				},
				legend: {   
				    	position: "ne",            
            				noColumns: 0
				}
			});
			current_zoom = "#currentday";
			
			// set a new cookie with zoom
			set_cookie("zoom", current_zoom, 1);
		});
		$("<div id='showvalue'></div>").css({
			position: "absolute",
			display: "none",
			border: "1px solid #fdd",
			padding: "2px",
			"background-color": "#fee",
			opacity: 0.80
		}).appendTo("body");

		$("#placeholder").bind("plothover", function (event, pos, item) {

			if (item) {
				var dateMillis = item.datapoint[0],
					value = item.datapoint[1].toFixed(2);
					
				var date = new Date(dateMillis);

				$("#showvalue").html("value: " + value + ", date: " + date.toLocaleString())
					.css({top: item.pageY+5, left: item.pageX+5})
					.fadeIn(200);
			} else {
				$("#showvalue").hide();
			}
		});
	});
	

	</script>

	<div id="content" style="position:absolute;left:1px;top:-15px;width:790px;height:622px;">

		<div class="demo-container" style="position:absolute;left:1px;top:0px;width:790px;height:322px;">
			<div id="placeholder" class="demo-placeholder"></div>
		</div>
		
		<br>
		<div id="data" class="data" style="position:absolute;left:1px;top:343px;width:790px;height:75px;">Display data: </div>
		<div id="sources" class="sources" style="position:absolute;left:1px;top:379px;width:790px;height:75px;">Display sources: </div>
		<div id="buttons" style="position:absolute;left:1px;top:414px;width:790px;height:150px;">
			<p>Zoom to: <button id="whole">Whole period</button>
			<button id="lastmonth">Last month</button>
			<button id="currentmonth">Current month</button>
			<button id="lastweek">Last seven days</button>
			<button id="currentday">Current day</button></p>
		</div>
		

	</div>

