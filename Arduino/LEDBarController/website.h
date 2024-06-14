const char* content = R"=====(<style type="text/css">
    th{
        text-align: right;
    }
    td{
        text-align: center;
    }
</style>
<h1>LED Bar Configuration</h1>
<table>
    <form>
        <tr>
            <th>
                <label for="dmxAdress">DMX Start-Adress:</label>
            </th>
            <td>
                <label for="dmxAdress">[1-[maxAdress]]</label>
            </td>
            <td>
                <input type="number" name="dmxAdress" id="dmxAdress" min="0" max="[maxAdress]" value="[dmxAdress]" />
            </td>
        </tr>
        <tr>
            <th>
                <label for="segments">Number of Segments:</label>
            </th>
            <td>
                <label for="segments">[1-[numLeds]]</label>
            </td>
            <td>
                <input type="number" name="segments" id="segments" min="1" max="[numLeds]" value="[segments]" />        
            </td>    
        </tr>
        <tr>
            <th>
                <label>DMX Footprint (inclusive):</label>
            </th>
            <td>
                
            </td>
            <td>
                <label>[startAdress]-[endAdress]</label>
            </td>
        </tr>
        [keepWebSrvInput]
        <tr>
            <td colspan="3">
                <input type="submit" name="submit" style="width: 100%;">
            </td>
        </tr>
    </form>
    <form>
        <tr>
            <td colspan="3">
                <input type="submit" name="identify" style="width: 100%;" value="Identify">
            </td>
        </tr>
    </form>
</table>)=====";

const char* keepWebServerInput = R"=====(
        <tr>
            <th>
                <label for="keepWebSrv">Keep Webserver:</label>
            </th>
            <td>
                <label for="keepWebSrv"></label>
            </td>
            <td>
                <input type="checkbox" name="keepWebSrv" id="keepWebSrv" [keepWebSrv]>
            </td>
        </tr>)=====";