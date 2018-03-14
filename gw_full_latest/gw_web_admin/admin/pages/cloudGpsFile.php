<!-- Tab panes -->
								
	<div class="tab-pane fade" id="cloudGpsFile-pills">
		</br>
		<div id="cloudGpsFile_status_msg"></div>
		
		<div class="col-md-10 col-md-offset-0">
		  <div class="table-responsive">
			<table class="table table-striped table-bordered table-hover">
			  <thead></thead>
			 <tbody>
			   <tr>
				<td>Enabled</td>
				<td id="cloudGpsFile_status_value"><?php cloud_status($clouds, "python CloudGpsFile.py"); ?></td>
				<td align="right"><button id="btn_edit_cloudGpsFile_status" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
				<td id="td_edit_cloudGpsFile_status">
					<div id="div_cloudGpsFile_status_options" class="form-group">					
						<div class="radio">
							<label>
								<input type="radio" name="optionsRadios" id="cloudGpsFile_true" value="true" checked>True
							</label>
							</br>
							<label>
								<input type="radio" name="optionsRadios" id="cloudGpsFile_false" value="false" >False
							</label>
						</div>
					</div>
				</td> 
				<td id="td_cloudGpsFile_status_submit" align="right">
					<button id="btn_cloudGpsFile_status_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>
			 </tr>
			<tr>
				<td>project name</td>
				<td id="cloudGpsFile_project_name_value"><?php echo $key_clouds['gpsfile_project_name']; ?></td>
				<td align="right"><button id="btn_edit_cloudGpsFile_project_name" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
				<td id="td_edit_cloudGpsFile_project_name">
					<div id="div_cloudGpsFile_project_name" class="form-group">
					    		<label>Project name</label>
                                			<input id="cloudGpsFile_project_name_input" class="form-control" placeholder="e.g. waziup" type="text" value="" autofocus>
					</div>
				</td>
				<td id="td_cloudGpsFile_project_name_submit" align="right">
					<button id="btn_cloudGpsFile_project_name_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>
			</tr>
			<tr>
				<td>organization name</td>
				<td id="cloudGpsFile_organization_name_value"><?php echo $key_clouds['gpsfile_organization_name']; ?></td>
				<td align="right"><button id="btn_edit_cloudGpsFile_organization_name" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
				<td id="td_edit_cloudGpsFile_organization_name">
					<div id="div_cloudGpsFile_organization_name" class="form-group">
					    		<label>Organization name</label>
                                			<input id="cloudGpsFile_organization_name_input" class="form-control" placeholder="e.g. UPPA" type="text" value="" autofocus>
					</div>
				</td>
				<td id="td_cloudGpsFile_organization_name_submit" align="right">
					<button id="btn_cloudGpsFile_organization_name_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>			
			</tr>
			<tr>
				<td>sensor name</td>
				<td id="cloudGpsFile_sensor_name_value"><?php echo $key_clouds['gpsfile_sensor_name']; ?></td>
				<td align="right"><button id="btn_edit_cloudGpsFile_sensor_name" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
				<td id="td_edit_cloudGpsFile_sensor_name">
					<div id="div_cloudGpsFile_sensor_name" class="form-group">
					    		<label>Sensor name</label>
                                			<input id="cloudGpsFile_sensor_name_input" class="form-control" placeholder="e.g. Sensor" type="text" value="" autofocus>
					</div>
				</td>
				<td id="td_cloudGpsFile_sensor_name_submit" align="right">
					<button id="btn_cloudGpsFile_sensor_name_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>			
			</tr>
			<tr>
				<td>source list</td>
				<td id="cloudGpsFile_source_list_value">
					<?php 
						$array = $key_clouds['gpsfile_source_list'];
						$size = count($key_clouds['gpsfile_source_list']);
						$i = 0;
						if($size == 0) echo "Empty";
						foreach( $array as &$cel){
							if(($size -1) == $i ){
								echo $cel ;							
							}else{
								echo $cel . ", ";
							}
							$i++;
						}
					?>
				</td>
				<td align="right"><button id="btn_edit_cloudGpsFile_source_list" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
				<td id="td_edit_cloudGpsFile_source_list">
					<div id="div_cloudGpsFile_source_list" class="form-group">
					    		<label>Add a sensor to your list</label>
                                			<input id="cloudGpsFile_source_list_input" class="form-control" placeholder="e.g. 6,7,8" type="text" 
							value="<?php 
								$array = $key_clouds['gpsfile_source_list'];
								$size = count($key_clouds['gpsfile_source_list']);
								$i = 0;
								if($size == 0) echo "Empty";
								foreach( $array as &$cel){
									if(($size -1) == $i ){
										echo $cel ;							
									}else{
										echo $cel . ", ";
									}
									$i++;
								}
							?>"
					 autofocus>
					</div>
				</td>
				<td id="td_cloudGpsFile_source_list_submit" align="right">
					<button id="btn_cloudGpsFile_source_list_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>
			</tr>
			<tr>
				<td>active interval</td>
				<td id="cloudGpsFile_active_interval_value"><?php echo $key_clouds['gpsfile_active_interval_minute']; ?></td>
				<td align="right"><button id="btn_edit_cloudGpsFile_active_interval" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
				<td id="td_edit_cloudGpsFile_active_interval">
					<div id="div_cloudGpsFile_active_interval" class="form-group">
					    		<label>Interval in minutes</label>
                                			<input id="cloudGpsFile_active_interval_input" class="form-control" placeholder="20" type="text" value="" autofocus>
					</div>
				</td>
				<td id="td_cloudGpsFile_active_interval_submit" align="right">
					<button id="btn_cloudGpsFile_active_interval_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>
			</tr>
			<tr>
				<td>SMS</td>
				<td id="cloudGpsFile_sms_value">
				<?php
					 if( $key_clouds['gpsfile_sms'] == 1){
						echo "Enabled";
					} else{
						echo "Disabled";				
					}
				?>
				</td>
				<td align="right"><button id="btn_edit_cloudGpsFile_sms" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
				<td id="td_edit_cloudGpsFile_sms">
					<div id="div_cloudGpsFile_sms" class="form-group">
							<label>
								<input type="radio" name="optionsRadios" id="cloudGpsFile_sms_true" value="true" checked>Enable
							</label>
							</br>
							<label>
								<input type="radio" name="optionsRadios" id="cloudGpsFile_sms_false" value="false" >Disable
							</label>
					</div>
				</td>
				<td id="td_cloudGpsFile_sms_submit" align="right">
					<button id="btn_cloudGpsFile_sms_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>
			</tr>
			<tr>
				<td>Pin code</td>
				<td id="cloudGpsFile_pin_value"><?php echo $key_clouds['gpsfile_pin']; ?></td>
				<td align="right"><button id="btn_edit_cloudGpsFile_pin" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
				<td id="td_edit_cloudGpsFile_pin">
					<div id="div_cloudGpsFile_pin" class="form-group">
					    		<label>Pin code</label>
                                			<input id="cloudGpsFile_pin_input" class="form-control" placeholder="Write pin code" type="text" value="" autofocus>
					</div>
				</td>
				<td id="td_cloudGpsFile_pin_submit" align="right">
					<button id="btn_cloudGpsFile_pin_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>
			</tr>
			<tr>
				<td>Contacts</td>
				<td id="cloudGpsFile_contacts_value">
					<?php 
						$array = $key_clouds['gpsfile_contacts'];
						$size = count($key_clouds['gpsfile_contacts']);
						$i = 0;
						if($size == 0) echo "Empty";
						foreach( $array as &$cel){
							if(($size -1) == $i ){
								echo $cel ;							
							}else{
								echo $cel . ", ";
							}
							$i++;
						}					
					?>
				</td>
				<td align="right"><button id="btn_edit_cloudGpsFile_contacts" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
				<td id="td_edit_cloudGpsFile_contacts">
					<div id="div_cloudGpsFile_contacts" class="form-group">
					    		<label>Add a mobile phone number your list</label>
                                			<input id="cloudGpsFile_contacts_input" class="form-control" placeholder="e.g. +33645783476" type="text" 
							value="<?php 						
										$array = $key_clouds['gpsfile_contacts'];
										$size = count($key_clouds['gpsfile_contacts']);
										$i = 0;
										if($size == 0) echo "Empty";
										foreach( $array as &$cel){
											if(($size -1) == $i ){
												echo $cel ;							
											}else{
												echo $cel . ", ";
											}
											$i++;
										}	
					 				?>" 
							autofocus>
					</div>
				</td>
				<td id="td_cloudGpsFile_contacts_submit" align="right">
					<button id="btn_cloudGpsFile_contacts_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>
			</tr>

			 </tbody>
			</table>
			<p>The sensor name will be <tt>project_name+'_'+organization_name+'_'+sensor_name+device_addr</tt>. For instance, for sensor 2 hosted by UPPA from waziup project, the sensor name will be <tt>waziup_UPPA_Sensor2</tt>.</p>
			
			<p><tt>active_interval</tt> defines the time window to mark remote GPS devices as active. It is expressed in minutes.</p>
			
			<p><b>Do not use space</b> in any of these parameters, use '_' or '-' instead.</p>
		  </div>
		</div>
</div>		
