// トリミングサイズ
var trimSize = 200;
var isImageOk = false;

var trimRect = {
        x : 0,
        y : 0,
        width : 0,
        height : 0,
        setStart : function(x, y) {
                this.x = x;
                this.y = y;
        },
        setEnd : function(x, y) {
                         if (this.x > x) {
                                 var tmp_x = this.x;
                                 this.x = x;
                                 x = tmp_x;
                         }
                         if (this.y > y) {
                                 var tmp_y = this.y;
                                 this.y = y;
                                 y = tmp_y;
                         }
                        this.width = x - this.x;
                        this.height = y - this.y;
        }
};

// 読み込んだイメージのサイズ

var canvas;
var context;
var img;


// トリム結果の表示先
var trimCanvas;

//////////////////

var live_flag = true;
var live_img;
var arrayBuffer;
var ws;

function setUp()
{
        live_img = document.getElementById("live_img");

        // Canvas
        canvas = document.getElementById('view_canvas');
        canvas.width = live_img.width;
        canvas.height = live_img.height;

        trimCanvas = document.createElement("CANVAS");
        trimCanvas.width = trimSize;
        trimCanvas.height = trimSize;

        // 画像
        img = new Image(live_img.width, live_img.height);
        img.onload = function() {
                context = canvas.getContext('2d');
                draw(context, 0,0);
        }


        ///////////////////////////////////////////////


        //WebSocketでサーバに接続
        ws = new WebSocket("ws://" + window.location.hostname + ":" + window.location.port + "/camera"); ws.binaryType = 'arraybuffer';   //受診するデータがバイナリであるので設定

        ws.onopen = function(){console.log("connection was established");};  //接続が確立した時に呼ばれる
        ws.onmessage = function(evt){
                if (!live_flag) {
                        return;
                }
                arrayBuffer = evt.data;
                //受信したデータを復号しbase64でエンコード
                live_img.src = "data:image/jpeg;base64," + encode(new Uint8Array(arrayBuffer));
        };

        window.onbeforeunload = function(){
                //ウィンドウ（タブ）を閉じたらサーバにセッションの終了を知らせる
                ws.close(1000);
        };
}

var canvasMouseDownState = 0;
function onCanvasMouseDown(event) {
        if (canvasMouseDownState == 0) {
                var rect = event.target.getBoundingClientRect();
                trimRect.setStart(event.clientX - rect.left, event.clientY - rect.top);
                draw2(context, trimRect.x, trimRect.y);
                canvasMouseDownState = 1;
        } else {
                var rect = event.target.getBoundingClientRect();
                trimRect.setEnd(event.clientX - rect.left, event.clientY - rect.top);
                draw3(context, trimRect);
                canvasMouseDownState = 0;
                isImageOk = true;
                trimImage();
        }
}

function draw(context, x, y) {
        if(typeof x === 'undefined') x = 0;
        if(typeof y === 'undefined') y = 0;
        context.drawImage(img, x, y, img.width, img.height);
}

function draw2(context, x, y) {
        draw(context);

        context.beginPath();
        context.strokeStyle = "rgb(0,0,0)";
        context.arc(x, y, 2, 0, Math.PI * 2, false);
        context.stroke();

        context.strokeStyle = "rgb(255,255,255)";
        context.arc(x, y, 4, 0, Math.PI * 2, false);
        context.stroke();
}

function draw3(context, rect) {
        draw(context);

        context.strokeStyle = 'rgb(0,0,0)';
        context.strokeRect(rect.x - 1, rect.y - 1, rect.width + 2, rect.height + 2);
        context.strokeStyle = 'rgb(255,255,255)';
        context.strokeRect(rect.x - 2, rect.y - 2, rect.width + 4, rect.height + 4);
}

function trimImage() {
        var trimData = context.getImageData(trimRect.x, trimRect.y, trimRect.width, trimRect.height);
        var trimContext = trimCanvas.getContext('2d');
        trimCanvas.width = trimRect.width;
        trimCanvas.height = trimRect.height;
        trimContext.putImageData(trimData, 0, 0 );
}

function EncodeHTMLForm( data )
{
    var params = [];

    for( var name in data )
    {
        var value = data[ name ];
        var param = encodeURIComponent( name ) + '=' + encodeURIComponent( value );

        params.push( param );
    }

    return params.join( '&' ).replace( /%20/g, '+' );
}
///////////////////////////

function encode (input) {
    var keyStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    var output = "";
    var chr1, chr2, chr3, enc1, enc2, enc3, enc4;
    var i = 0;

    while (i < input.length) {
        chr1 = input[i++];
        chr2 = i < input.length ? input[i++] : Number.NaN; // Not sure if the index
        chr3 = i < input.length ? input[i++] : Number.NaN; // checks are needed here

        enc1 = chr1 >> 2;
        enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
        enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
        enc4 = chr3 & 63;

        if (isNaN(chr2)) {
            enc3 = enc4 = 64;
        } else if (isNaN(chr3)) {
            enc4 = 64;
        }
        output += keyStr.charAt(enc1) + keyStr.charAt(enc2) +
                  keyStr.charAt(enc3) + keyStr.charAt(enc4);
    }
    return output;
}


function takeImage()
{
        isImageOk = false;
        img.src = live_img.src;
}

function playPauseImage()
{
        live_flag = !live_flag;
}

function register()
{
        var name = document.getElementById("name").value;
        var ledonoff = getRadiobuttonValue("ledonoff");
        var ledcolor = getRadiobuttonValue("ledcolor");
        var ir_signal = document.getElementById("ir_signal").value;
        if (!isImageOk) {
                alert("error: image");
                return;
        }
        if (!name.match(/\S/g)) {
                alert("error: name");
                return;
        }
        if (!ir_signal.match(/\S/g)) {
                alert("error: IR signal");
                return;
        }
        if (!ledonoff) {
                alert("error: LED onoff");
                return;
        }
        if (!ledcolor) {
                alert("error: LED color");
                return;
        }

        var base64= trimCanvas.toDataURL('image/png');
        var xmlHttpRequest = new XMLHttpRequest();
        xmlHttpRequest.open( 'POST', '/add' );
        xmlHttpRequest.onreadystatechange = function(){
                if (xmlHttpRequest.readyState === 4 && xmlHttpRequest.status === 200){
                        location.href = '/';
                }
        }
        xmlHttpRequest.setRequestHeader( 'Content-Type', 'application/x-www-form-urlencoded' );

        xmlHttpRequest.send( EncodeHTMLForm({p: "register", name: name, ir_signal: ir_signal, ledonoff: ledonoff, ledcolor: ledcolor, img: base64.replace(/^.*,/, '')}) );
}

function irReceiveOnReadyStateChange(xmlHttpRequest)
{
        if (xmlHttpRequest.readyState === 4 && xmlHttpRequest.status === 200){
                var data = xmlHttpRequest.responseText;
                var textarea = document.getElementById("ir_signal");
                textarea.value = data;
                document.getElementById("ir_signal_button").disabled = false;
        }
}

function irReceive()
{
        document.getElementById("ir_signal_button").disabled = true;
        var xmlHttpRequest = new XMLHttpRequest();
        xmlHttpRequest.open( 'POST', '/add' );
        xmlHttpRequest.onreadystatechange = function() {irReceiveOnReadyStateChange(xmlHttpRequest) };
        xmlHttpRequest.setRequestHeader( 'Content-Type', 'application/x-www-form-urlencoded' );
        xmlHttpRequest.send( EncodeHTMLForm({p: "ir"}) );
}

function getRadiobuttonValue(name)
{
        var radioList = document.getElementsByName(name);
        for(var i=0; i<radioList.length; i++){
                if (radioList[i].checked) {
                        return radioList[i].value;
                }
        }

        return undefined;
}

