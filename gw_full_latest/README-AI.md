LoRa Gateway AI Framework
=========================

This framework is an interface or tool that allows users to build and use Artificial Intelligence
models easily on LoRa Gateway, without getting into the depth of the underlying algorithms and runtime environment setup. It's based on a Jupyter-Lab Docker container.  

<img src="https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/AI-Picture6.png" alt="AI Framework" width="400"/> 

The LoRa Gateway AI Framework is based on a layered architecture where each layer depends on the previous layer. There are 7 layers, as follows:

* Hardware
* Operating System
* Containerization Platform
* Artificial Intelligence Container
* Runtime Engine
* Packages
* LoRa Gateway AI Notebooks

Features
--------

* In-browser editing for code, with automatic syntax highlighting, indentation, and tab completion/introspection.
* Execute code from the browser, with results of computations attached to the code which generated them.
* Displaying result of computation using rich media representations, such as HTML, LaTeX, PNG, SVG, etc.
* In-browser editing using Markdown markup language, which can provide commentary for the code, is not limited to plain text.
* The ability to easily include mathematical notation within Markdown language using LaTeX.

How to Access LoRa Gateway AI Framework
------------------------------------------

1. Load the LoRa Gateway web interface
   `http://<LoRa Gateway IP address>`  
![LoRa Homepage](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/AI-Picture1.png)

2. Click on Web Admin Interface
![LoRa Admin Link](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/AI-Picture2.png)

3. Enter Admin Credentials
   Default credentials: **admin / loragateway**  
![Credentials input](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/AI-Picture3.png)

4. Web Admin Landing Page
   Click on Jupyter Link on the left aside bar  
![LoRa Admin GUI](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/AI-Picture4.png)

5. Jupyter Landing Page
   Create or open a Notebook of your preference. Traverse directories using the left panel navigator.
![Jupyter Notebook](https://github.com/CongducPham/LowCostLoRaGw/blob/master/images/AI-Picture5.png)

**Notebooks are saved in `/home/pi/lora_gateway/AI/notebooks` directory. To import Notebooks, they must be placed in this directory to show up on Jupyter.**
