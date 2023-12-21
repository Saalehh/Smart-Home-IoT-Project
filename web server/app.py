from flask import Flask, render_template, request, flash, url_for, redirect, session, jsonify
from datetime import timedelta
from database import *
import os
from dotenv import load_dotenv
import requests
import json


# Load environment variables from the .env file
load_dotenv()

ESP32_IP_ADDRESS = "192.168.43.168"

app = Flask(__name__, template_folder="templates")
app.config['DEBUG'] = True
app.config["SECRET_KEY"] = os.environ.get("SECRET_KEY")
app.permanent_session_lifetime = timedelta(minutes=8)

connection = create_sqlite_connection()


# Handle 404 Not Found error
@app.errorhandler(404)
def page_not_found(error):
    return render_template('404.html'), 404


# Handle 500 Internal Server Error
@app.errorhandler(500)
def internal_server_error(error):
    return render_template('500.html'), 500


@app.route("/", methods=["GET"])
def home():
    if "user_id" in session:
        user_id = session["user_id"]
        user = get_user_data_from_db(id=user_id)
        warnings = get_all_warnings()
        if any(warning['status'] == 0 for warning in warnings):
            warn = True
        else:
            warn = False

        # URL of the esp server
        url = "http://" + ESP32_IP_ADDRESS
        response = requests.get(url)

        response_str = response.content.decode()
        status = json.loads(response_str)
        return render_template("home.html",
                               user=user,
                               warn=warn,
                               status=status,
                               ESP32_IP_ADDRESS=ESP32_IP_ADDRESS)
    else:
        flash("You are not logged in", category="error")
        return redirect(url_for("login"))


@app.route("/sign-up", methods=["GET", "POST"])
def sign_up():
    if request.method == "GET":
        return render_template("sign_up.html")
    elif request.method == "POST":
        if is_user_exist_in_db(username=request.form.get("username")):
            flash(
                "The username you have entered already exists, choose different one",
                category="error",
            )
            return render_template("sign_up.html")

        if request.form.get("password1") != request.form.get("password2"):
            flash("The passwords should match each others", category="error")
            return render_template("sign_up.html")

        data = {
            "username": request.form.get("username"),
            "password": hashlib.md5(
                request.form.get("password1").encode("utf-8")
            ).hexdigest()
        }
        if add_user_to_db(data) != -1:
            flash(
                "Congratulations! your account has been created successfully",
                category="success",
            )
            return redirect(url_for("login"))
        else:
            flash(
                "There was an error occurred while creating your account",
                category="error",
            )
            return redirect(url_for("home"))


@app.route("/login", methods=["GET", "POST"])
def login():
    if request.method == "GET":
        return render_template("login.html")
    elif request.method == "POST":
        if check_password(
                username=request.form.get("username"),
                password=request.form.get("password"),
        ):
            session.permanent = True
            username = request.form.get("username")
            user_id = get_user_data_from_db(username=username)["id"]

            session["username"] = username
            session["user_id"] = user_id
            flash("Logged In Successfully", category="success")
            warnings = get_all_warnings()
            if any(warning['status'] == 0 for warning in warnings):
                warn = True
            else:
                warn = False
            return redirect(url_for("home", warn=warn))
        else:
            flash(
                "The password or username you entered is not correct",
                category="error",
            )
            return render_template("login.html")


@app.route("/add-warning")
def add_warning():
    warn_type = request.args.get("warning")

    if warn_type == "intrusion":
        wrong_password_attempt_warning()
        warn_response = {"status": "success"}
    else:
        warn_response = {"status": "error"}
 
    return jsonify(warn_response)


@app.route("/warnings")
def show_warnings():
    if not "user_id" in session:
        flash("Login is required", "error")
        return redirect(url_for("login"))

    warnings = get_all_warnings()
    if any(warning['status'] == 0 for warning in warnings):
        warn = True
    else:
        warn = False

    warnings_seen()
    return render_template(
        "warnings.html",
        warnings=warnings,
        warn=warn,
        user=get_user_data_from_db(id=session["user_id"]),
    )


@app.route("/turn-lights-on")
def turn_lights_on_man():
    url = f"http://{ESP32_IP_ADDRESS}/lights-on"
    response = requests.get(url)
    return redirect(url_for("home"))


@app.route("/turn-lights-off")
def turn_lights_off_man():
    url = f"http://{ESP32_IP_ADDRESS}/lights-off"
    response = requests.get(url)
    return redirect(url_for("home"))


@app.route("/turn-fan-on")
def turn_fan_on_man():
    url = f"http://{ESP32_IP_ADDRESS}/fan-on"
    response = requests.get(url)
    return redirect(url_for("home"))


@app.route("/turn-fan-off")
def turn_fan_off_man():
    url = f"http://{ESP32_IP_ADDRESS}/fan-off"
    response = requests.get(url)
    return redirect(url_for("home"))


@app.route("/auto-control")
def set_auto_control():
    url = f"http://{ESP32_IP_ADDRESS}/auto-control"
    response = requests.get(url)
    return redirect(url_for("home"))


@app.route("/reset")
def reset():
    url = f"http://{ESP32_IP_ADDRESS}/reset"
    response = requests.get(url)
    return redirect(url_for("home"))


if __name__ == '__main__':
    init_db(connection)
    app.run(host='0.0.0.0', port=80)
