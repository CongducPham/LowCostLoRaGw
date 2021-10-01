<?php
include_once '../libs/php/functions.php';

// begin our session
session_start();

// check if the user is logged out
if(!isset($_SESSION['username'])){
                header('Location: login.php');
                exit();
}

require 'header.php';
?>


            <div class="navbar-default sidebar" role="navigation">
                <!-- <div class="sidebar-nav navbar-collapse collapse in" aria-expanded="true"> -->
                <div class="sidebar-nav navbar-collapse collapse" aria-expanded="false" role="navigation">
                    <ul class="nav" id="side-menu">
                        <li>
                            <a href="cloud.php"><i class="fa fa-cloud"></i> Clouds</a>
                        </li>
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

        <!-- 20200730 stagiaire -->
        <script>
            $('button.navbar-toggle').css('display', 'block');
            $('button.navbar-toggle').attr('aria-expanded', '"false"');
            //$('button.navbar-toggle').attr('aria-expanded', 'true');

            $(document).ready(function () {
                $('.sidebar-nav.navbar-collapse').on('hidden.bs.collapse', function () {
                //$('.sidebar-nav.navbar-collapse').on('hide.bs.collapse', function () {
                    $('#page-wrapper').toggleClass('toggled');
                });

                //$('.sidebar-nav.navbar-collapse').on('shown.bs.collapse', function () {
                $('.sidebar-nav.navbar-collapse').on('show.bs.collapse', function () {
                    $('#page-wrapper').toggleClass('toggled');
                });

                $('.sidebar-nav.navbar-collapse').collapse('hide');

            }); // ready

        </script>
        <div id="page-wrapper" class="toggled">
            <div class="embed-responsive embed-responsive-16by9">
                <iframe src="/jupyter" class="embed-responsive-item" allowfullscreen></iframe>
            </div>
            <!-- #embed-responsive -->

<?php require 'footer.php'; ?>
