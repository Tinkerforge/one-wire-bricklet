Imports System
Imports System.Threading
Imports Tinkerforge

Module ExampleReadDS18B20Temperature
    Const HOST As String = "localhost"
    Const PORT As Integer = 4223
    Const UID As String = "XYZ" ' Change XYZ to the UID of your One Wire Bricklet

    Sub Main()
        Dim ipcon As New IPConnection() ' Create IP connection
        Dim ow As New BrickletOneWire(UID, ipcon) ' Create device object

        ipcon.Connect(HOST, PORT) ' Connect to brickd
        ' Don't use device before ipcon is connected

        ow.WriteCommand(0, 78) ' WRITE SCRATCHPAD
        ow.Write(0) ' ALARM H (unused)
        ow.Write(0) ' ALARM L (unused)
        ow.Write(127) ' CONFIGURATION: 12 bit mode

        ' Read temperature 10 times
        Dim i As Integer
        For i = 0 To 9
            ow.WriteCommand(0, 68) ' CONVERT T (start temperature conversion)
            Thread.Sleep(1000) ' Wait for conversion to finish
            ow.WriteCommand(0, 190) ' READ SCRATCHPAD

            Dim tLow, tHigh, status As Integer
            ow.Read(tLow, status)
            ow.Read(tHigh, status)

            Dim temperature As Single = tLow or (tHigh << 8)

            ' Negative 12-bit values are sign-extended to 16-bit two's complement
            If temperature > 1 << 12 Then
                temperature -= 1 << 16
            End If

            ' 12-bit mode measures in units of 1/16°C.
            Console.WriteLine("Temperature: " + (temperature/16.0).ToString() + " °C")
        Next i

        Console.WriteLine("Press key to exit")
        Console.ReadLine()
        ipcon.Disconnect()
    End Sub
End Module
