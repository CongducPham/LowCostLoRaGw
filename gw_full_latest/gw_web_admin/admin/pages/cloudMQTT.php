<!-- Tab panes -->
								
	<div class="tab-pane fade" id="cloudMQTT-pills">
		</br>
		<div id="cloudMQTT_status_msg"></div>
		
		<div class="col-md-10 col-md-offset-0">
		  <div class="table-responsive">
			<table class="table table-striped table-bordered table-hover">
			  <thead></thead>
			 <tbody>
				<tr>
					<td>Enabled</td>
					<td id="cloudMQTT_status_value"><?php cloud_status($clouds, "python CloudMQTT.py"); ?></td>
					<td align="right"><button id="btn_edit_cloudMQTT_status" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
					<td id="td_edit_cloudMQTT_status">
						<div id="div_cloudMQTT_status_options" class="form-group">
							
							<div class="radio">
								<label>
									<input type="radio" name="optionsRadios" id="cloudMQTT_true" value="true" checked>True
								</label>
								</br>
								<label>
									<input type="radio" name="optionsRadios" id="cloudMQTT_false" value="false" >False
								</label>
							</div>
						</div>
					</td> 
					<td id="td_cloudMQTT_status_submit" align="right">
						<button id="btn_cloudMQTT_status_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
					</td>
				</tr>
				<tr>
					<td>MQTT Server</td>
					<td id="td_cloudMQTT_MQTT_server_value">
						<?php 
							echo $key_clouds['mqtt_server'];
						?>
					</td> 
					<td align="right">
						<button id="btn_edit_cloudMQTT_MQTT_server" type="button" class="btn btn-primary">
							<span class="fa fa-edit" ></span>
						</button>
					</td>
					<td id="td_edit_cloudMQTT_MQTT_server">
						<div id ="div_cloudMQTT_MQTT_server" class="form-group">
							<label>MQTT server</label>
							<input id="cloudMQTT_MQTT_server_input" class="form-control" placeholder="e.g. test.mosquitto.org" type="text" value="" ></input>
						</div>
					</td>
					<td id="td_cloudMQTT_MQTT_server_submit">
						<button id="btn_cloudMQTT_MQTT_server_submit" type="submit" class="btn btn-primary">
						Submit
							<span class="fa fa-arrow-right"></span>
						</button>
					</td>
				</tr>
				<tr>
					<td>project name</td>
					<td id="td_cloudMQTT_project_name_value">
						<?php 
							echo $key_clouds['mqtt_project_name'];
						?>
					</td>
					<td align="right">
						<button id="btn_edit_cloudMQTT_project_name" type="button" class="btn btn-primary">
							<span class="fa fa-edit" ></span>
						</button>
					</td>
					<td id="td_edit_cloudMQTT_project_name">
						<div id ="div_cloudMQTT_project_name" class="form-group">
							<label>Project name</label>
							<input id="cloudMQTT_project_name_input" class="form-control" placeholder="e.g. waziup" type="text" value="" ></input>
						</div>
					</td>
					<td id="td_cloudMQTT_project_name_submit">
						<button id="btn_cloudMQTT_project_name_submit" type="submit" class="btn btn-primary">
						Submit
							<span class="fa fa-arrow-right"></span>
						</button>
					</td>
				</tr>
				<tr>
					<td>organization name</td>
					<td id="td_cloudMQTT_organization_name_value">
						<?php 
							echo $key_clouds['mqtt_organization_name'];
						?>
					</td>
					<td align="right">
						<button id="btn_edit_cloudMQTT_organization_name" type="button" class="btn btn-primary">
							<span class="fa fa-edit" ></span>
						</button>
					</td>
					<td id="td_edit_cloudMQTT_organization_name">
						<div id ="div_cloudMQTT_organization_name" class="form-group">
							<label>Organization name</label>
							<input id="cloudMQTT_organization_name_input" class="form-control" placeholder="e.g. UPPA" type="text" value="" ></input>
						</div>
					</td>
					<td id="td_cloudMQTT_organization_name_submit">
						<button id="btn_cloudMQTT_organization_name_submit" type="submit" class="btn btn-primary">
						Submit
							<span class="fa fa-arrow-right"></span>
						</button>
					</td>
				</tr>
				<tr>
					<td>sensor name</td>
					<td id="td_cloudMQTT_sensor_name_value">
						<?php 
							echo $key_clouds['mqtt_sensor_name'];
						?>
					</td>
					<td align="right">
						<button id="btn_edit_cloudMQTT_sensor_name" type="button" class="btn btn-primary">
							<span class="fa fa-edit" ></span>
						</button>
					</td>
					<td id="td_edit_cloudMQTT_sensor_name">
						<div id ="div_cloudMQTT_sensor_name" class="form-group">
							<label>Sensor name</label>
							<input id="cloudMQTT_sensor_name_input" class="form-control" placeholder="e.g. Sensor" type="text" value="" ></input>
						</div>
					</td>
					<td id="td_cloudMQTT_sensor_name_submit">
						<button id="btn_cloudMQTT_sensor_name_submit" type="submit" class="btn btn-primary">
						Submit
							<span class="fa fa-arrow-right"></span>
						</button>
					</td>
				</tr>
				<tr>
					<td>source list</td>
					<td id="td_cloudMQTT_source_list_value">
						<?php 
							$array = $key_clouds['mqtt_source_list'];
							$size = count($key_clouds['mqtt_source_list']);
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
					<td align="right">
						<button id="btn_edit_cloudMQTT_source_list" type="button" class="btn btn-primary">
							<span class="fa fa-edit" ></span>
						</button>
					</td>
					<td id="td_edit_cloudMQTT_source_list">
						<div id ="div_cloudMQTT_source_list" class="form-group">
							<label>Add a sensor to your list</label>
							<input id="cloudMQTT_source_list_input" class="form-control" placeholder="e.g. 6,7,8" type="text" 
							value="<?php 
								$array = $key_clouds['mqtt_source_list'];
								$size = count($key_clouds['mqtt_source_list']);
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
					<td id="td_cloudMQTT_source_list_submit">
						<button id="btn_cloudMQTT_source_list_submit" type="submit" class="btn btn-primary">
						Submit
							<span class="fa fa-arrow-right"></span>
						</button>
					</td>
				</tr>
			 </tbody>
			</table>
					<p>The MQTT cloud will build a topic name using the following format: <tt>project_name/organization_name/sensor_name+device_addr/nomenclature</tt>. For instance, sensor 2 hosted by <tt>UPPA</tt> from <tt>waziup</tt> project and sending "<tt>TC/22.5/HU/54</tt>" will publish both <tt>waziup/UPPA/Sensor2/TC</tt> and <tt>waziup/UPPA/Sensor2/HU</tt> topics. You can subscribe to <tt>waziup/UPPA/Sensor2/#</tt> to cover all levels from <tt>waziup/UPPA/Sensor2</tt>.</p>
		
		<p><b>Do not use space</b> in any of these parameters, use '_' or '-' instead.</p>
					
		  </div>
		</div>
</div>		
