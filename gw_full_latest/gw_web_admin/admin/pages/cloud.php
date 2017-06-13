<?php
include_once '../libs/php/functions.php';

$clouds = null; $encrypted_clouds= null; $lorawan_encrypted_clouds = null;
process_clouds_json($clouds, $encrypted_clouds, $lorawan_encrypted_clouds);

require 'header.php';
?>



            <div class="navbar-default sidebar" role="navigation">
                <div class="sidebar-nav navbar-collapse">
                    <ul class="nav" id="side-menu">
                        </br>
                        
                        <li>
                            <a href="gateway_config.php"><i class="fa fa-dashboard fa-fw"></i> Gateway Configuration</a>
                        </li>
                        <li>
                            <a href="gateway_update.php"><i class="fa fa-dashboard fa-fw"></i> Gateway Update</a>
                        </li>
                        <li>
                            <a href="system.php"><i class="fa fa-dashboard fa-fw"></i> System</a>
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
                           
                        </div>
                        <!-- /.panel-heading -->
                        <div class="panel-body">
                            <!-- Nav tabs -->
                            <ul class="nav nav-pills">
                                <li class="active"><a href="#thingSpeak-pills" data-toggle="tab">ThingSpeak</a>
                                </li>
                                <li><a href="#waziup-pills" data-toggle="tab">Waziup Orion</a>
                                </li>
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
                                                    		<input type="radio" name="optionsRadios" id="thingspeak_false" value="false" checked>False
                                                		</label>
                                            		</div>
                                        		</div>
                                        	</td> 
   										    <td id="td_thingspeak_status_submit" align="right">
   										    		<button id="btn_thingspeak_status_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
										 </tbody>
    								    </table>
    							      </div>
    							    </div>
    							    
                                	</br>
                                	
                                	</br>
                                	
                                    <div class="col-md-8 col-md-offset-1"> 
                    						<div class="panel-body">
                        						<form id="thingspeak_form" role="form">
                            						<fieldset>
                                						<div class="form-group">
                                							<label>Write Key</label>
                                							<input class="form-control" placeholder="Write key" name="write_key" type="text" value="" autofocus>
                                						</div>

                                						<center>
                                							<button  type="submit" class="btn btn-primary">Submit</button>
                                							<button  id="btn_thingspeak_form_reset" type="reset" class="btn btn-primary">Clear</button>
                                						</center> 
                            						</fieldset>
                        						</form>
                    						</div>
            						</div>	
            						</br>
            					
            						<div id="thingspeak_form_msg"></div>
            						
                                </div>
                                
                                <!-- tab-pane -->
                                
                                <div class="tab-pane fade" id="waziup-pills">
                                    </br>
                                    <div id="waziup_status_msg"></div>
            						
            						<div class="col-md-10 col-md-offset-0">
                                      <div class="table-responsive">
										<table class="table table-striped table-bordered table-hover">
   										  <thead></thead>
										 <tbody>
										   <tr>
    									    <td>Enabled</td>
    										<td id="waziup_status_value"><?php cloud_status($clouds, "python CloudWAZIUP.py"); ?></td>
    										<td align="right"><button id="btn_edit_waziup_status" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_waziup_status">
   										    	<div id="div_waziup_status_options" class="form-group">
                                            		
                                           			<div class="radio">
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="waziup_true" value="true" checked>True
                                                		</label>
                                                		</br>
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="waziup_false" value="false" checked>False
                                                		</label>
                                            		</div>
                                        		</div>
                                        	</td> 
   										    <td id="td_waziup_status_submit" align="right">
   										    		<button id="btn_waziup_status_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
										 </tbody>
    								    </table>
    							      </div>
    							    </div>
    							    
                                	</br>
                                	
                                	</br>
                                    
                                    <div class="col-md-8 col-md-offset-1"> 
                    						<div class="panel-body">
                        						<form id="waziup_form" role="form">
                            						<fieldset>
                                						<div class="form-group">
                                							<label>Project</label>
                                							<input class="form-control" placeholder="project name" name="project_name" type="text" value="" autofocus>
                                						</div>
                                						<div class="form-group">
                                							<label>Organization</label>
                                							<input class="form-control" placeholder="Organization name" name="organization_name" type="text" value="" autofocus>
                                						</div>
                                						<div class="form-group">
                                							<label>Service tree</label>
                                							<input class="form-control" placeholder="Service tree" name="service_tree" type="text" value="" autofocus>
                                						</div>

                                						<center>
                                							<button  type="submit" class="btn btn-primary">Submit</button>
                                							<button  id="btn_waziup_form_reset" type="reset" class="btn btn-primary">Clear</button>
                                						</center> 
                            						</fieldset>
                        						</form>
                    						</div>
            						</div>	
            						</br>
            						<div id="waziup_form_msg"></div>
                                	
                                </div>
                                <!-- tab-pane --> 
                                
                                <!-- tab-pane -->  
                        </div>        
                        <!-- /.panel-body -->
                        
                </div>
                <!-- /.panel -->
        
<?php require 'footer.php'; ?>
