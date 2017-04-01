<!--
*
* Copyright (C) 2016 Nicolas Bertuol, University of Pau, France
*
* nicolas.bertuol@etud.univ-pau.fr
*
-->

<?php 
header('Content-Type: application/csv');
header('Content-Disposition: attachment; filename=example.csv');
header('Pragma: no-cache');
$connection = new MongoClient();
$collection = $connection->messages->ReceivedData;

$cursor = $collection->find();
$cursor->sort(array('time' => 1));

//echo columns name first
echo ("type;gateway_eui;node_eui;snr;rssi;cr;datarate;time;data"."\n");

//then echo each part of each document from the collection
foreach($cursor as $cur)
   echo ($cur['type'].";"
		.$cur['gateway_eui'].";"
		.$cur['node_eui'].";"
		.$cur['snr'].";"
		.$cur['rssi'].";"
		.$cur['cr'].";"
		.$cur['datarate'].";"
		.gmdate('Y-m-d H:i:s', $cur['time']->sec).";"
		.$cur['data'].";"
		."\n");
?>