<?php
include_once '../libs/php/functions.php';

// begin our session
session_start();

// check if the user is logged out
if(!isset($_SESSION['username'])){
                header('Location: login.php');
                exit();
}

$clouds = null; $encrypted_clouds= null; $lorawan_encrypted_clouds = null; $key_clouds = null;

process_clouds_json($clouds, $encrypted_clouds, $lorawan_encrypted_clouds);

$key_clouds = process_key_clouds();

$key_waziup_file = "/home/pi/lora_gateway/key_WAZIUP.py";	
$waziup = is_file($key_waziup_file);

require 'header.php';
?>
            <div class="navbar-default sidebar" role="navigation">
                <div class="sidebar-nav navbar-collapse">
                    <ul class="nav" id="side-menu">
                        </br>
                        
                        <li>
                            <a href="gateway_config.php"><i class="fa fa-edit"></i> Gateway Configuration</a>
                        </li>
                        <li>
                            <a href="gateway_update.php"><i class="fa fa-upload"></i> Gateway Update</a>
                        </li>
                        <li>
                            <a href="system.php"><i class="fa fa-linux"></i> System</a>
                        </li>
                        <li>
                            <a href="jupyter.php"><img src="jupyter-logo.svg" width="15" height="14"> Jupyter</a>
                        </li>                        
                    </ul>
                </div>
                <!-- /.sidebar-collapse -->
            </div>
            <!-- /.navbar-static-side -->
        </nav>

        <div id="page-wrapper">
            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Cloud</h1>
                </div>
            </div>
            <!-- /.row -->
          
            <div class="panel panel-default">
                        <div class="panel-heading">
                          	<div id="cloud_msg"></div>
                        </div>
                        <!-- /.panel-heading -->
                        <div class="panel-body">
                            <!-- Nav tabs -->
                            <ul class="nav nav-pills">
								<?php
									if($waziup){	
                            			echo '<li class="active"><a href="#waziup-pills" data-toggle="tab">Cloud WAZIUP</a></li>';
                            			echo '<li><a href="#thingSpeak-pills" data-toggle="tab">ThingSpeak</a></li>';
									}
									else {
										echo '<li class="active"><a href="#thingSpeak-pills" data-toggle="tab">ThingSpeak</a></li>';
									}
								?>
								<li><a href="#cloudNoInternet-pills" data-toggle="tab">Cloud No Internet</a>
								</li>
								<li><a href="#cloudGpsFile-pills" data-toggle="tab">Cloud Gps File</a>
								</li>
								<li><a href="#cloudMQTT-pills" data-toggle="tab">Cloud MQTT</a>
								</li>
								<li><a href="#cloudNodeRed-pills" data-toggle="tab">Cloud Node-RED </a>
								</li>
								<li><a href="#cloudTTN-pills" data-toggle="tab">Cloud TTN </a>
								</li>								
                            </ul>
							</br>
            				<p>&nbsp;&nbsp;&nbsp;&nbsp;When enabling a new cloud, you need to reboot for changes to take effect.</p>
            				<p>&nbsp;&nbsp;&nbsp;&nbsp;It is possible to change a cloud parameter at run-time although it is recommended to reboot.</p>            				
                            
                            <!-- Tab panes -->
                            <div class="tab-content">
								<?php 
									if($waziup){
										echo '<div class="tab-pane fade in active" id="waziup-pills">';
                                		require 'waziup.php';
                                		echo '</div>';
                                		echo '<div class="tab-pane fade" id="thingSpeak-pills">';
									}
									else {
										echo '<div class="tab-pane fade in active" id="thingSpeak-pills">';
									}
								?>
                                	</br>
                                	<div id="thingspeak_status_msg"></div>
            						
            						<div class="col-md-10 col-md-offset-0">
            						
										<?php
											$date=date('Y-m-d\TH:i:s');
											echo '<p>Date/Time: ';
											echo $date;
											echo '</p>';									
											ob_start();
											system("tac /home/pi/lora_gateway/log/post-processing.log | egrep -a -m 1 'uploading with python.*CloudThingSpeak.*py' | cut -d '>' -f1"); 
											//system("egrep -a 'uploading with python.*CloudThingSpeak.*py' /home/pi/lora_gateway/log/post-processing.log | tail -1 | cut -d '>' -f1");
											$last_upload=ob_get_contents(); 
											ob_clean();
											echo '<p>';
											if ($last_upload=='') {
												echo '<font color="red"><b>no upload with CloudThingSpeak.py found</b></font>';					
											}
											else {
												$date=str_replace("T", " ", $date, $count);
												$datetime1 = new DateTime($date);
												$last_upload_1=str_replace("T", " ", $last_upload, $count);
												$datetime2 = new DateTime($last_upload_1);
												$interval = $datetime1->diff($datetime2);
												echo 'last upload time with CloudThingSpeak.py: <font color="green"><b>';
												echo $last_upload;
												echo $interval->format(' %mm-%dd-%hh-%imin from current date');
												echo '</b></font>';					
											}
											echo '</p>';									
										?>                            
           						
                                      <div class="table-responsive">
										<table class="table table-striped table-bordered table-hover">
   										  <thead></thead>
										 <tbody>
										   <tr>
    									    <td>Enabled
												<?php 
													ob_start(); 
													$server="https://api.thingspeak.com";
													system("wget -q --tries=3 --spider $server", $retval);
													$msg=ob_get_contents(); 
													ob_clean();
													if ($retval == 0) {
														echo ' <font color="green"><b>[server online]</b></font>';					
													}
													else {
														echo ' <font color="red"><b>[server offline]</b></font>';					
													}				 
												?>    									    
    									    </td>
    										<td id="thingspeak_status_value"></td>
    										
											<td align="right">
											<input type="checkbox" id="thingspeak_status_toggle" data-toggle="toggle" data-on="true" data-off="false" <?php if(get_cloud_status($clouds, "python CloudThingSpeak.py")) echo "checked";?>>											
											</td>
			
											<!-- <td align="right"><button id="btn_edit_thingspeak_status" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td> --> 
   										   	
   										   	<td id="td_edit_thingspeak_status">
   										    	<div id="div_thingspeak_status_options" class="form-group">
                                           			<div class="radio">
                                           			<fieldset id="thingspeak_status_group" >
                                                		<label>
                                                    		<input type="radio" name="thingspeak_status_group" id="thingspeak_true" value="true" checked>True
                                                		</label>
                                                		</br>
                                                		<label>
                                                    		<input type="radio" name="thingspeak_status_group" id="thingspeak_false" value="false" >False
                                                		</label>
                                                		</fieldset>
                                            		</div>
                                        		</div>
                                        	</td> 
   										    <td id="td_thingspeak_status_submit" align="right">
   										    		<button id="btn_thingspeak_status_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
   										   
   										   <tr>
    									    <td>Write Key</td>
    										<td id="write_key_value"><?php echo $key_clouds['thingspeak_channel_key']; ?></td>
    										<td align="right"><button id="btn_edit_write_key" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_write_key">
   										    	<div id="div_update_write_key" class="form-group">
                                            		<label>Write Key</label>
                                					<input id="write_key_input" class="form-control" placeholder="Write key" name="write_key" type="text" value="" autofocus>
                                        		</div>
                                        	</td> 
   										    <td id="td_write_key_submit" align="right">
   										    		<button id="write_key_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   	</tr>
											
											<tr>
												<td><a href="#" class="my_tooltip" data-toggle="tooltip" title="Indicate a list of accepted device addresses, e.g. 6,7,0x01020304">source list</a></td>
												<td id="td_thingspeak_source_list_value">
													<?php 
														$array = $key_clouds['thingspeak_source_list'];
														$size = count($key_clouds['thingspeak_source_list']);
														$i = 0;
														if($size == 0) echo "Empty";
														foreach( $array as $cel){
															if(($size-1) == $i){
																echo $cel;
															}else{
																echo $cel . ",";
															}
															$i++;
														}
													?>
												</td>
												<td align="right">
													<button id="btn_edit_thingspeak_source_list" type="button" class="btn btn-primary">
														<span class="fa fa-edit"></span>
													</button>
												</td>
												<td id="td_edit_thingspeak_source_list">
													<div id="div_edit_thingspeak_source_list" class="form-group">
														<label>Add a sensor to your list</label>
														<input id="thingspeak_source_list_input" class="form-control" placeholder="e.g. 6,7,0x01020304" type="text"
														value="<?php 
															$array = $key_clouds['thingspeak_source_list'];
															$size = count($key_clouds['thingspeak_source_list']);
															$i = 0;
															foreach( $array as $cel){
																if(($size-1) == $i){
																	echo $cel;
																}else{
																	echo $cel . ",";
																}
																$i++;
															}
														?>" autofocus>
													</div>
												</td>
												<td id="td_thingspeak_source_list_submit" align="right">
													<button id="btn_thingspeak_source_list_submit" type="submit" class="btn btn-primary">
														Submit
														<span class="fa fa-arrow-right"></span>
													</button>
												</td>
											</tr>
										 </tbody>
    								    </table>
    							      </div>
    							    </div>
    							    
                                </div>
                                <!-- tab-pane -->

                                <?php require 'cloudNoInternet.php'; ?>
                                <?php require 'cloudGpsFile.php'; ?> 
                                <?php require 'cloudMQTT.php'; ?>
                                <?php require 'cloudNodeRed.php'; ?>
                      			<?php require 'cloudTTN.php'; ?>
                                <!-- tab-pane -->  
                        </div>        
                        <!-- /.panel-body -->
                </div>
                <!-- /.panel -->
		</div>
	</div>        
<?php require 'footer.php'; ?>
