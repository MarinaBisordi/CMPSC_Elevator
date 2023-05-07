from jupyter_dash import JupyterDash
from dash import html, dcc, dash_table


#Instantiation
app = JupyterDash(__name__)

app.layout = html.Div(style={'backgroundColor':'#deb887','padding':'30px'},children=[
    html.Div([
        html.H1('Elevator Status', style={'backgroundColor':'#cd5c5c','padding':'20px'})
              ]),
    html.Div(style={'backgroundColor':'#e9967a','padding':'30px'},children=[
        html.Iframe(src="https://thingspeak.com/channels/2136533/charts/1?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&type=line&update=15",
                style={'display': 'block','border-style':'none',"width": "700px", "height":"400px", "border": "1px solid"},
                allow="fullscreen")
    ])
])
         

#Execution
app.run_server(debug=False, mode='external', port=8060)




