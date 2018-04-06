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
                            </ul>

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
                                      <div class="table-responsive">
										<table class="table table-striped table-bordered table-hover">
   										  <thead></thead>
										 <tbody>
										   <tr>
    									    <td>Enabled</td>
    										<td id="thingspeak_status_value"><?php cloud_status($clouds, "python CloudThingSpeak.py"); ?></td>
    										<td align="right"><button id="btn_edit_thingspeak_status" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_thingspeak_status">
   										    	<div id="div_thingspeak_status_options" class="form-group">
                                            		
                                           			<div class="radio">
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="thingspeak_true" value="true" checked>True
                                                		</label>
                                                		</br>
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="thingspeak_false" value="false" >False
                                                		</label>
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
												<td>source list</td>
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
														<input id="thingspeak_source_list_input" class="form-control" placeholder="Write source addresses" type="text"
														value="<?php 
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
                      			
                                <!-- tab-pane -->  
                        </div>        
                        <!-- /.panel-body -->
                </div>
                <!-- /.panel -->
		</div>
	</div>        
<?php require 'footer.php'; ?>
