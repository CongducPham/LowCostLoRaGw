<?php 
header('Content-Type: application/csv');
// 20201109 engineer
// header('Content-Disposition: attachment; filename=example.csv');
header('Content-Disposition: attachment; filename=received_data.csv');
header('Pragma: no-cache');

/* 20201109 engineer
$connection = new MongoClient();
$collection = $connection->messages->ReceivedData;

$cursor = $collection->find();
$cursor->sort(array('time' => 1));
*/

try {

	$mng = new MongoDB\Driver\Manager("mongodb://localhost:27017");
	$query = new MongoDB\Driver\Query([], ['sort' => [ 'time' => 1]]); 
	$data = $mng->executeQuery("messages.ReceivedData", $query);

} catch (MongoDB\Driver\Exception\Exception $e) {

	$filename = basename(__FILE__);

	echo "The $filename script has experienced an error.\n"; 
	echo "It failed with the following exception:\n";

	echo "Exception:", $e->getMessage(), "\n";
	echo "In file:", $e->getFile(), "\n";
	echo "On line:", $e->getLine(), "\n";       
}



//echo columns name first
echo ("type;gateway_eui;node_eui;snr;rssi;cr;datarate;time;data"."\n");

//then echo each part of each document from the collection

// 20201109 engineer
//foreach($cursor as $cur)
$it = new \IteratorIterator($data);
$it->rewind(); // Very important;
while ($cursor = $it->current()) {
   $cur = get_object_vars($cursor);
   echo ($cur['type'].";"
		.$cur['gateway_eui'].";"
		.$cur['node_eui'].";"
		.$cur['snr'].";"
		.$cur['rssi'].";"
		.$cur['cr'].";"
		.$cur['datarate'].";"
// 20201109 engineer
//		.gmdate('Y-m-d H:i:s', $cur['time']->sec).";"
		.gmdate('Y-m-d H:i:s', $cur['time']->toDateTime()->getTimestamp()).";"
		.$cur['data'].";"
		."\n");

   $it->next();
}
?>
