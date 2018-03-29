<!-- Tab panes -->
								
	<div class="tab-pane fade" id="cloudNoInternet-pills">
		</br>
		<div id="cloudNoInternet_status_msg"></div>
		
		<div class="col-md-10 col-md-offset-0">
		  <div class="table-responsive">
			<table class="table table-striped table-bordered table-hover">
			  <thead></thead>
			 <tbody>
			   <tr>
				<td>Enabled</td>
				<td id="cloudNoInternet_status_value"><?php cloud_status($clouds, "python CloudNoInternet.py"); ?></td>
				<td align="right"><button id="btn_edit_cloudNoInternet_status" type="button" class="btn btn-primary"><span class="fa fa-edit"></span></button></td>
				<td id="td_edit_cloudNoInternet_status">
					<div id="div_cloudNoInternet_status_options" class="form-group">
						
						<div class="radio">
							<label>
								<input type="radio" name="optionsRadios" id="cloudNoInternet_true" value="true" checked>True
							</label>
							</br>
							<label>
								<input type="radio" name="optionsRadios" id="cloudNoInternet_false" value="false" >False
							</label>
						</div>
					</div>
				</td> 
				<td id="td_cloudNoInternet_status_submit" align="right">
					<button id="btn_cloudNoInternet_status_submit" type="submit" class="btn btn-primary">Submit <span class="fa fa-arrow-right"></span></button>
				</td>
			   </tr>
			 </tbody>
			</table>
			<p>Cloud No Internet is used with WAZIUP cloud only.</pp>
		  </div>
		</div>
</div>		
