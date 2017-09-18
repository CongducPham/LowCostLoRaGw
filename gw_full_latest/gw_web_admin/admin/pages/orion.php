<?php

?>

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
    										<td id="waziup_status_value"><?php cloud_status($clouds, "python CloudOrion.py"); ?></td>
    										<td align="right"><button id="btn_edit_waziup_status" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_waziup_status">
   										    	<div id="div_waziup_status_options" class="form-group">
                                            		
                                           			<div class="radio">
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="waziup_true" value="true" checked>True
                                                		</label>
                                                		</br>
                                                		<label>
                                                    		<input type="radio" name="optionsRadios" id="waziup_false" value="false" >False
                                                		</label>
                                            		</div>
                                        		</div>
                                        	</td> 
   										    <td id="td_waziup_status_submit" align="right">
   										    		<button id="btn_waziup_status_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
   										   
   										   <tr>   
    									    <td>Project</td>
    										<td id="project_value"><?php echo $key_clouds['project_name']; ?></td>
    										<td align="right"><button id="btn_edit_project" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_project">
   										    	<div id="div_update_project" class="form-group">
                                            		<label>Project</label>
                                					<input id="project_input" class="form-control" placeholder="Project name" name="project_name" type="text" value="" autofocus>
                                        		</div>
                                        	</td> 
   										    <td id="td_project_submit" align="right">
   										    		<button id="project_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
   										   
   										   <tr>
    									    <td>Organization</td>
    										<td id="organization_value"><?php echo $key_clouds['organization_name']; ?></td>
    										<td align="right"><button id="btn_edit_organization" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_organization">
   										    	<div id="div_update_organization" class="form-group">
                                            		<label>Organization</label>
                                					<input id="organization_input" class="form-control" placeholder="Organization name" name="organization_name" type="text" value="" autofocus>
                                        		</div>
                                        	</td> 
   										    <td id="td_organization_submit" align="right">
   										    		<button id="organization_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
   										   
   										   <tr>
    									    <td>Service tree</td>
    										<td id="service_tree_value"><?php echo $key_clouds['service_tree']; ?></td>
    										<td align="right"><button id="btn_edit_service_tree" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_service_tree">
   										    	<div id="div_update_service_tree" class="form-group">
                                            		<label>Service tree</label>
                                					<input id="service_tree_input" class="form-control" placeholder="Service tree" name="service_tree" type="text" value="" autofocus></div>
                                        	</td> 
   										    <td id="td_service_tree_submit" align="right">
   										    		<button id="service_tree_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>

   										   <tr>
    									    <td>Orion token</td>
    										<td id="orion_token_value"><?php echo $key_clouds['orion_token']; ?></td>
    										<td align="right"><button id="btn_edit_orion_token" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
   										   	<td id="td_edit_orion_token">
   										    	<div id="div_update_orion_token" class="form-group">
                                            		<label>Orion token</label>
                                					<input id="orion_token_input" class="form-control" placeholder="Orion token" name="orion_token" type="text" value="" autofocus></div>
                                        	</td> 
   										    <td id="td_orion_token_submit" align="right">
   										    		<button id="orion_token_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
   										    </td>
   										   </tr>
   										      										   
										 </tbody>
    								    </table>
    									<p>The WAZIUP cloud uses the FIWARE Orion broker. The <b>FIWARE service</b> will be the project name and the <b>FIWARE service-path></b> will obtained from <tt> '/'+organization+service-tree</tt></p>
    								  	<p>The device id will be <tt>organization+"Sensor"+device_addr</tt>. For instance, from sensor 2 hosted by UPPA: <tt>UPPA_Sensor2</tt></p>
    							      </div>
    							    </div>
		
		
