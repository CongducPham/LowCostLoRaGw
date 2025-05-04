<?php
 
// Fonction de selection des fichiers suivant les extensions predeterminées avec $extensions
function addphoto($dir,$extensions,$photos=array()){
	
	foreach($extensions as $ext){
		$addphoto = glob($dir . '/*.'.$ext); // selectionne les photos contenues du dossier
		usort($addphoto, create_function('$a,$b', 'return filemtime($b) - filemtime($a);'));
		$photos =  array_merge($photos, $addphoto); // ajoute les photos au tableau de resultats
	}
	return $photos;
}

// Fonction de parcours du dossier
function scandir_through($dir,$extensions,$photos=array()){

    $photos = addphoto($dir,$extensions,$photos); // lance la fonction addphoto de selection des photos du dossier (concerne le dossier principal, si il contient pas d'images, tu peux supprimer cette ligne)
	$items = glob($dir . '/*'); // selectionne le contenu de $dir

    for ($i = 0; $i < count($items); $i++) { // parcours du contenu de $dir
        if (is_dir($items[$i])) { // si le contenu est un dossier		
			$photos = addphoto($items[$i],$extensions,$photos); // lance la fonction addphoto de selection des photos du dossier	
			scandir_through($items[$i],$extensions,$photos); // lance la fonction scandir_through pour parcourir le dossier
        }
    }
    return $photos;
}

function display_last_capture_for_all_nodes($main_dir, $extensions){
	//$extensions = array('bmp');
	echo '<table class="table table-striped table-bordered table-hover table-condensed">';
	echo '<tr>';
	for ($i = 2; $i <= 255; $i++){
		$dir = $main_dir.''.$i; // example node_1 directory for storing images from video node 1
		//echo $dir;
		
		if(is_dir($dir)){
			//echo '<table border=1>';
			//echo '<tr>';
		
			foreach(scandir_through($dir,$extensions) as $key=>$filename){
				echo '<td>';
					echo 'Node '.$i.'</br>';
					echo date('F d Y h:i A',filemtime($filename)).'</br>';
					echo '<img src="'.$filename.'" />';
				echo '</td>';
				break;
			}
			//echo '</tr>';
			//echo '</table>';
		}	
	}
	echo '</tr>';
	echo '</table>';
}


function display_pic($filename){
	echo '<td>';
		echo date('F d Y h:i A',filemtime($filename)).'</br>';
		echo '<img src="'.$filename.'" />';
		//echo '<center><img src="'.$filename.'" /></center>';
	echo '</td>';
}


function display_captures_per_node($main_dir, $extensions, $node, $time_interval){
	
	$dir = $main_dir.''.$node;
	
	if(is_dir($dir)){
		echo '</br>';
		//echo 'Node '.$node.'</br></br>';
		echo '<table class="table table-striped table-bordered table-hover table-condensed">';
		echo '<tr>';
		
		foreach(scandir_through($dir,$extensions) as $key=>$filename){
			$t = time()-filemtime($filename);
			//echo "duree = ".$t;
			switch ($time_interval) {
    			case "1h":
        			if($t <= 3600)
        				display_pic($filename);
        			break;
    			case "2h":
        			if($t <= 2 * 3600)
        				display_pic($filename);
        			break;
    			case "3h":
        			if($t <= 3 * 3600)
        				display_pic($filename);
        			break;
        		case "all":
        			display_pic($filename);
        			break;
			}
			
		}
		echo '</tr>';
		echo '</table>';
	}
		
}

?>
