<?php
include_once '../libs/php/functions.php';

$radio_conf= null; $gw_conf= null; $alert_conf = null;
process_gw_conf_json($radio_conf, $gw_conf, $alert_conf);

require 'header.php';
?>



            <div class="navbar-default sidebar" role="navigation">
                <div class="sidebar-nav navbar-collapse">
                    <ul class="nav" id="side-menu">
                        </br>
                        
                        <li>
                            <a href="cloud.php"><i class="fa fa-dashboard fa-fw"></i> Cloud</a>
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
                    <h1 class="page-header">Gateway configuration</h1>
                </div>
            </div>
            <!-- /.row -->
          
            <div class="panel panel-default">
                        <div class="panel-heading">
                           Configuration
                        </div>
                        <!-- /.panel-heading -->
                        <div class="panel-body">
                            <!-- Nav tabs -->
                            <ul class="nav nav-pills">
                                <li class="active"><a href="#radio_conf-pills" data-toggle="tab">Radio</a>
                                </li>
                                <li><a href="#gw_conf-pills" data-toggle="tab">Gateway</a>
                                </li>
                                <li><a href="#alert_conf-pills" data-toggle="tab">Alert</a>
                                </li>
                            </ul>

                            <!-- Tab panes -->
                            <div class="tab-content">
                            	<div class="tab-pane fade in active" id="radio_conf-pills">
                                    </br></br>
                                    
                                    <div id="radio_msg"></div>
                                    
                                    <div class="col-md-10 col-md-offset-0">
                                      <div class="table-responsive">
										<table class="table table-striped table-bordered table-hover">
   										  <thead>
    								       
    									 </thead>
										<tbody>
										   <tr>
    									    <td>Mode</td>
    										<td id="mode_value"><?php echo $radio_conf['mode']; ?></td>
    										<td align="right"><button id="btn_edit_mode" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										    <td id="td_edit_mode">
   										    	<div id="div_mode_select" class="form-group">
                                            		<label>Select a mode</label>
                                            		<select id="mode_select" class="form-control">
                                                		<option selected>1</option> <option>2</option> <option>3</option> <option>4</option>
                                                		<option>5</option> <option>6</option> <option>7</option> <option>8</option>
                                                		<option>9</option> <option>10</option>
                                           			</select>
                                        		</div>
                                        	</td> 
   										    <td id="mode_submit" align="right">
   										    		<button id="btn_edit_mode" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>	
   										   </tr>
   					
   										   <tr>
    									    <td>Frequency</td>
    										<td id="freq_value"><?php echo $radio_conf['freq']; ?></td>
    										<td align="right"><button id="btn_edit_freq" type="button" class="btn btn-primary" href="gateway.php?freq="><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_freq">
   										    	<div class="form-group">
                                            		<label>ISM Band </label>
                                            		<select id="band_freq_select" class="form-control">
                                                		<option></option> <option>433MHz</option> <option>868MHz</option> <option>915MHz</option> 
                                           			</select>
                                           			
                                           			<label>Frequency</label>
                                            		<select id="freq_select" class="form-control">
                                                		
                                           			</select>
                                        		</div>
                                        	</td>
                                        	<td id="freq_submit" align="right"><button id="btn_edit_mode" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button></td> 
   										   </tr>
										</tbody>
    								  </table>
    							     </div>
    							     
    							  </div>
    							                           		
                                </div>
                                <!-- tab-pane -->
                                
                                <div class="tab-pane fade" id="gw_conf-pills">
                                    </br>
                                    <!-- <h4>Gateway settings</h4></br> -->
                                    <div id="gw_msg"></div>
                                    <!-- Tab panes -->
                
                                    <div class="col-md-10 col-md-offset-0">
                                      <div class="table-responsive">
										<table class="table table-striped table-bordered table-hover">
   										  <thead>
    								       
    									 </thead>
										 <tbody>
										   <tr>
    									    <td>Gateway ID</td>
    										<td id="gw_id_value"><?php echo $gw_conf['gateway_ID']; ?></td>
    										<td align="right"><button id="btn_edit_gw_id" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_gw_id">
   										    	<div id="div_update_gw_id" class="form-group">
                                            		<label>Gateway ID</label>
													<input id="gw_id_input" class="form-control" placeholder="New value" type="text" value="" autofocus>
                                        		</div>
                                        	</td> 
   										    <td id="td_gw_id_submit" align="right">
   										    		<button id="gw_id_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   
   										   </tr>
   										   <tr>
    									    <td>AES</td>
    										<!-- <td id="aes_value"><?php echo $gw_conf['aes']; ?></td> -->
    										<td id="aes_value">
    											<?php
    												if($gw_conf['aes'])
													{
    													echo "true";    
													}
													else {
   					 									echo "false";   
													}
    											?>
    										</td>
    										<td align="right"><button id="btn_edit_aes" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_aes">
   										    	<div id="div_aes_options" class="form-group">
                                            		<label></label>
                                           			<div class="radio">
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="aes_true" value="true" checked>True
                                                		</label>
                                                		</br>
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="aes_false" value="false" checked>False
                                                		</label>
                                            		</div>
                                        		</div>
                                        	</td> 
   										    <td id="td_aes_submit" align="right">
   										    		<button id="btn_aes_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
										 </tbody>
    								    </table>
    							      </div>
    							    </div>
    							
                                </div>
                                <!-- tab-pane -->
                                
                                <div class="tab-pane fade" id="alert_conf-pills">
                                    </br>
                                    <div id="alert_msg"></div>
                                 <div class="col-md-7 col-md-offset-0">
                                    
                                    <div class="table-responsive">
										<table class="table table-striped table-bordered table-hover">
   										  <thead>
    								       
    									 </thead>
										<tbody>
										   <tr>
    									    <td>Mail</td>
    										<!-- <td id="use_mail_value"><?php echo $alert_conf['use_mail']; ?></td> -->
    										<td id="use_mail_value">
    											<?php
    												if($alert_conf['use_mail'])
													{
    													echo "true";    
													}
													else {
   					 									echo "false";   
													}
    											?>
    										</td>
    										<td align="right"><button id="btn_edit_use_mail" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_use_mail">
   										    	<div id="div_use_mail_options" class="form-group">
                                            		<label></label>
                                           			<div class="radio">
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="use_mail_true" value="true" checked>True
                                                		</label>
                                                		</br>
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="use_mail_false" value="false" checked>False
                                                		</label>
                                            		</div>
                                        		</div>
                                        	</td> 
   										    <td id="td_use_mail_submit" align="right">
   										    		<button id="btn_use_mail_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
   										   <tr>
    									    <td>SMS</td>
    										<!-- <td id="use_sms_value"><?php echo $alert_conf['use_sms']; ?></td> -->
    										<td id="use_sms_value">
    											<?php
    												if($alert_conf['use_sms'])
													{
    													echo "true";    
													}
													else {
   					 									echo "false";   
													}
    											?>
    										</td>
    										<td align="right"><button id="btn_edit_use_sms" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_use_sms">
   										    	<div id="div_use_sms_options" class="form-group">
                                            		<label></label>
                                           			<div class="radio">
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="use_sms_true" value="true" checked>True
                                                		</label>
                                                		</br>
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="use_sms_false" value="false" checked>False
                                                		</label>
                                            		</div>
                                        		</div>
                                        	</td> 
   										    <td id="td_use_sms_submit" align="right">
   										    		<button id="btn_use_sms_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
										</tbody>
    								 </table>
    							  </div>
                                
                                </div>  
                                    
                            </div>    
                                
                        <!-- /.panel-body -->
                        
                </div>
                <!-- /.panel -->
        
<?php require 'footer.php'; ?>
