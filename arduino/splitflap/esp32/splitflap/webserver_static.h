
#ifndef INDEX_HTML
// todo: fancy svelte thing

auto index_html = R"FOO(

<html>
<head>
    <title>Splitflap</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: sans-serif;
            font-size: 1.5em;
            margin: 0;
            padding: 0;
        }
        .container {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
        }
        .container > * {
            margin: 0.5em;
        }
        .container > .flap {
            display: flex;
            flex-direction: row;
            align-items: center;
            justify-content: center;
            width: 100%;
            height: 100%;
            background-color: #eee;
            border-radius: 0.5em;
        }
        .container > .flap > .flap__letter {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            width: 100%;
            height: 100%;
            background-color: #fff;
            border-radius: 0.5em;
        }
        .poem {
            font-family: monospace;
            font-size: 1em;
            margin: 0.5em;
            padding: 0.5em;
            border: 1px solid #ccc;
            border-radius: 0.5em;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="flap">
            <div class="flap__letter">A</div>
            <div class="flap__letter">B</div>
            <div class="flap__letter">C</div>
            <div class="flap__letter">D</div>
            <div class="flap__letter">E</div>
            <div class="flap__letter">F</div>
        </div>
    </div>

    <!-- form to set the text by POST to /set -->
    <form action="/set" method="post">
        <input type="text" name="message" placeholder="message">
        <input type="submit" value="Set">
    </form>

    <!-- form to reset the text by POST to /reset -->
    <form action="/reset" method="post">
        <input type="submit" value="Reset">
    </form>

    <!-- periodically get /text and update the flaps -->
    <script>
        const flapLetters = document.querySelectorAll('.flap__letter');
        const updateFlaps = () => {
            fetch('/text')
                .then(response => response.text())
                .then(text => {
                    for (let i = 0; i < flapLetters.length; i++) {
                        flapLetters[i].innerText = text[i];
                    }
                });
        };
        updateFlaps();
        setInterval(updateFlaps, 1000);
    </script>
    <div class="poem">
             O freddled gruntbuggly thy micturations are to me
                 As plured gabbleblochits on a lurgid bee.
              Groop, I implore thee my foonting turlingdromes.   
           And hooptiously drangle me with crinkly bindlewurdles,
Or I will rend thee in the gobberwarts with my blurlecruncheon, see if I don't.

                (by Prostetnic Vogon Jeltz; see p. 56/57)
    </div>
</body>
</html>
)FOO";

#endif // INDEX_HTML

