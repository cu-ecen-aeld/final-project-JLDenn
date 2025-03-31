
I will be making a toolbox inventory system controller device using buildroot on a Raspberry Pi4. I'm building a Bluetooth LE QR code scanner in my IoTEF (ECEN 5823) class that will act as a toolbox mounted QR scanner to scan in/out tools with QR codes on them as they are replaced/removed from a toolbox. I see this system possibly being used in an aircraft maintenance hanger or auto mechanic shop where tool control is very important. 

I plan to make the BLE central unit with a buildroot image running on the Pi4, where my application would be written in C and possibly using the btferret Bluetooth library (https://github.com/petzval/btferret), and would pair with the BLE QR code scanner and act as a local database that maintains the toolbox inventory internally and provides a user interface (likely a basic web page) to add/update/remove tools from the toolboxes. The eventual final concept being one Pi could connect to all of the toolboxes and maintain all of their inventories, and provide a central point to query for missing tools for all toolboxes.

>The controller (Raspberry Pi4) will:
> - Initially connect and pair with the BLE scanner (this will require user interaction during initial bring-up with fresh firmware)
> - It will provide a way for users to add/update/remove tools in the database (likely web interface). Entries will contain the tool QR code, the toolbox ID it will be kept in, a tool name, and the current checked in/out state.
> - Every time the scanner sends a check-in/out indication, the database will be updated and the Pi will respond with the latest number of tools currently removed from that toolbox so the scanner can display the outstanding tool count on its display.
> - There will be a way for the scanner to request an "end of day" report with the number of tools are checked out of that toolbox and would include the name of one of those tools to aid in locating it.


>The BLE scanner (Developed on the Blue Gecko dev board) will:<br>
> - Sleep until one of two buttons are pressed. 
> - On button press, the scanner will wake up and accept either a CHECK-IN or CHECK-OUT scan (depending on the button)
> - For ~5 seconds wait for a QR code scan using the QR Scanner I'm getting (https://www.sparkfun.com/useful-sensors-tiny-code-reader.html)
> - On scan, it will read the code and send an indication to the central (my Pi) with the toolbox ID, IN or OUT, and the tool code.
> - Display the feedback from the central with the total number of tools currently checked out of the toolbox.
> - Wait for another ~5 seconds for another tool scan
> - Return to sleep.
> As a fallback, I can simulate the BLE scanner functionality using Silicon Labs SI Connect app, so if my scanner project in my other class doesn't end up working (I don't expect I'll have an issue with it, I am LOVING that class, and I'm doing very well), I can still validate the tool control Pi system by using an emulated scanner.