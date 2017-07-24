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

$key_orion_file = "/home/pi/lora_gateway/key_Orion.py";	
$orion = is_file($key_orion_file);

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
                                <li class="active"><a href="#thingSpeak-pills" data-toggle="tab">ThingSpeak</a>
                                </li>
				<?php
					if($orion){	
                                		echo '<li><a href="#waziup-pills" data-toggle="tab">Waziup Orion</a></li>';
					}
				?>
                            </ul>

                            <!-- Tab panes -->
                            <div class="tab-content">
                                                            
                                <div class="tab-pane fade in active" id="thingSpeak-pills">
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
										 </tbody>
    								    </table>
    							      </div>
    							    </div>
    							    
                                </div>
                                
                                <!-- tab-pane -->
                               
				<?php 
					if($orion){
                                		require 'orion.php';
					}
				?>
    				
                                </div>
                                <!-- tab-pane --> 
                                
                                <!-- tab-pane -->  
                        </div>        
                        <!-- /.panel-body -->
                        
                </div>
                <!-- /.panel -->
        
<?php require 'footer.php'; ?>
