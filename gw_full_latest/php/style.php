<!--
*
* Copyright (C) 2016 Nicolas Bertuol, University of Pau, France
*
* nicolas.bertuol@etud.univ-pau.fr
*
-->

<?php
/*** set the content type header ***/
/*** Without this header, it wont work ***/
header("Content-type: text/css");


?>
<style>
body
{
   background-color: #FFFFFF;
   color: #000000;
   font-family: Arial;
   font-weight: normal;
   font-size: 13px;
   line-height: 1.1875;
   margin: 0;
   padding: 0;
}
#SlideMenu1
{
   overflow: scroll;
}
#SlideMenu1 ul
{
   list-style: none;
   margin: 0;
   padding: 0;
   font-family: Arial;
   font-weight: normal;
   font-size: 13px;
}
#SlideMenu1 li a
{
   background-color: #337AB7;
   border-color: #2E6DA4;
   border-style: solid;
   border-width: 1px;
   color: #FFFFFF;
   cursor: pointer;
   display: block;
   font-weight: normal;
   left: 0;
   height: 16px;
   margin: 0px 0px 0px 0px;
   padding: 2px 2px 2px 5px;
   position: relative;
   text-decoration: none;
   top: 0;
}
#SlideMenu1 li a:hover
{
   background-color: #286090;
   border-color: #204D74;
   color: #FFFFFF;
   font-weight: normal;
   text-decoration: none;
}
#SlideMenu1 li li p
{
   background-color: #DDDDFF;
   border-color: #8C8C8C;
   border-style: solid;
   border-width: 1px;
   color: #333333;
   display: block;
   font-weight: normal;
   height: 16px;
   margin: 1px 0px 1px 0px;
   padding: 2px 2px 2px 5px;
   text-decoration: none;
}
#Editbox1
{
   border: 1px #CCCCCC solid;
   -moz-border-radius: 4px;
   -webkit-border-radius: 4px;
   border-radius: 4px;
   background-color: #286090;
   border-color: #204D74;
   color: #FFFFFF;
   background-image: none;
   font-family: Arial;
   font-weight: normal;
   font-size: 13px;
   padding: 4px 4px 4px 4px;
   text-align: left;
   vertical-align: middle;
}
#Editbox1:focus
{
   border-color: #66AFE9;
   -webkit-box-shadow: inset 0px 1px 1px rgba(0,0,0,0.075), 0px 0px 8px rgba(102,175,233,0.60);
   -moz-box-shadow: inset 0px 1px 1px rgba(0,0,0,0.075), 0px 0px 8px rgba(102,175,233,0.60);
   box-shadow: inset 0px 1px 1px rgba(0,0,0,0.075), 0px 0px 8px rgba(102,175,233,0.60);
   outline: 0;
}
#wb_Form1
{
   background-color: #337AB7;
   border-color: #2E6DA4;
   background-image: none;
   border: 1px #101010 solid;
   -moz-border-radius: 4px;
   -webkit-border-radius: 4px;
   border-radius: 4px;
}
#banner
{
   background-color: #337AB7;
   border-color: #2E6DA4;
   background-image: none;
   border: 1px #101010 solid;
   -moz-border-radius: 4px;
   -webkit-border-radius: 4px;
   border-radius: 4px;
}
#banner h1
{
   color: #FFFFFF;
   display: block;
   font-weight: normal;
   font-size: 60px;
   position: relative;
   text-decoration: none;
   text-align: center;
   margin-top: 3%;

}
#Label1
{
   border: 0px #CCCCCC solid;
   -moz-border-radius: 4px;
   -webkit-border-radius: 4px;
   border-radius: 4px;
   background-color: transparent;
   background-image: none;
   color: #FFFFFF;
   font-family: Arial;
   font-weight: normal;
   font-size: 13px;
   padding: 4px 4px 4px 4px;
   text-align: left;
   vertical-align: middle;
}
#Combobox1
{
   border: 1px #CCCCCC solid;
   -moz-border-radius: 4px;
   -webkit-border-radius: 4px;
   border-radius: 4px;
   background-color: #286090;
   border-color: #204D74;
   color: #FFFFFF;
   background-image: none;
   font-family: Arial;
   font-weight: normal;
   font-size: 13px;
   padding: 4px 4px 4px 4px;
   -webkit-box-sizing: border-box;
   -moz-box-sizing: border-box;
   box-sizing: border-box;
}
#Combobox1:focus
{
   border-color: #66AFE9;
   -webkit-box-shadow: inset 0px 1px 1px rgba(0,0,0,0.075), 0px 0px 8px rgba(102,175,233,0.60);
   -moz-box-shadow: inset 0px 1px 1px rgba(0,0,0,0.075), 0px 0px 8px rgba(102,175,233,0.60);
   box-shadow: inset 0px 1px 1px rgba(0,0,0,0.075), 0px 0px 8px rgba(102,175,233,0.60);
   outline: 0;
}
#Label2
{
   border: 0px #CCCCCC solid;
   -moz-border-radius: 4px;
   -webkit-border-radius: 4px;
   border-radius: 4px;
   background-color: transparent;
   background-image: none;
   color: #FFFFFF;
   font-family: Arial;
   font-weight: normal;
   font-size: 13px;
   padding: 4px 4px 4px 4px;
   text-align: left;
   vertical-align: middle;
}
#Button1
{
   border: 1px #7FFFD4 solid;
   -moz-border-radius: 4px;
   -webkit-border-radius: 4px;
   border-radius: 4px;
   background-color: #286090;
   border-color: #204D74;
   color: #FFFFFF;
   cursor: pointer;
   background-image: none;
   font-family: Arial;
   font-weight: normal;
   font-size: 13px;
}
#Label3
{
   border: 0px #CCCCCC solid;
   -moz-border-radius: 4px;
   -webkit-border-radius: 4px;
   border-radius: 4px;
   background-color: transparent;
   background-image: none;
   color: #FFFFFF;
   font-family: Arial;
   font-weight: normal;
   font-size: 13px;
   padding: 4px 4px 4px 4px;
   text-align: left;
   vertical-align: middle;
}
#Combobox2
{
   border: 1px #CCCCCC solid;
   -moz-border-radius: 4px;
   -webkit-border-radius: 4px;
   border-radius: 4px;
   background-color: #286090;
   border-color: #204D74;
   color: #FFFFFF;
   background-image: none;
   font-family: Arial;
   font-weight: normal;
   font-size: 13px;
   padding: 4px 4px 4px 4px;
   -webkit-box-sizing: border-box;
   -moz-box-sizing: border-box;
   box-sizing: border-box;
}
#Combobox2:focus
{
   border-color: #66AFE9;
   -webkit-box-shadow: inset 0px 1px 1px rgba(0,0,0,0.075), 0px 0px 8px rgba(102,175,233,0.60);
   -moz-box-shadow: inset 0px 1px 1px rgba(0,0,0,0.075), 0px 0px 8px rgba(102,175,233,0.60);
   box-shadow: inset 0px 1px 1px rgba(0,0,0,0.075), 0px 0px 8px rgba(102,175,233,0.60);
   outline: 0;
}
a
{
   color: #0000FF;
   text-decoration: underline;
}
a:visited
{
   color: #800080;
}
a:active
{
   color: #FF0000;
}
a:hover
{
   color: #0000FF;
   text-decoration: underline;
}
h1
{
   font-family: Arial;
   font-weight: bold;
   font-size: 32px;
   font-style: normal;
   text-decoration: none;
   color: #000000;
   background-color: transparent;
   margin: 0px 0px 0px 0px;
   padding: 0px 0px 0px 0px;
   display: inline;
}
h2
{
   font-family: Arial;
   font-weight: bold;
   font-size: 27px;
   font-style: normal;
   text-decoration: none;
   color: #000000;
   background-color: transparent;
   margin: 0px 0px 0px 0px;
   padding: 0px 0px 0px 0px;
   display: inline;
}
h3
{
   font-family: Arial;
   font-weight: normal;
   font-size: 24px;
   font-style: normal;
   text-decoration: none;
   color: #000000;
   background-color: transparent;
   margin: 0px 0px 0px 0px;
   padding: 0px 0px 0px 0px;
   display: inline;
}
h4
{
   font-family: Arial;
   font-weight: normal;
   font-size: 21px;
   font-style: italic;
   text-decoration: none;
   color: #000000;
   background-color: transparent;
   margin: 0px 0px 0px 0px;
   padding: 0px 0px 0px 0px;
   display: inline;
}
h5
{
   font-family: Arial;
   font-weight: normal;
   font-size: 19px;
   font-style: normal;
   text-decoration: none;
   color: #000000;
   background-color: transparent;
   margin: 0px 0px 0px 0px;
   padding: 0px 0px 0px 0px;
   display: inline;
}
h6
{
   font-family: Arial;
   font-weight: normal;
   font-size: 16px;
   font-style: normal;
   text-decoration: none;
   color: #000000;
   background-color: transparent;
   margin: 0px 0px 0px 0px;
   padding: 0px 0px 0px 0px;
   display: inline;
}
.CustomStyle
{
   font-family: "Courier New";
   font-weight: bold;
   font-size: 16px;
   font-style: normal;
   text-decoration: none;
   color: #000080;
   background-color: transparent;
}
</style>
