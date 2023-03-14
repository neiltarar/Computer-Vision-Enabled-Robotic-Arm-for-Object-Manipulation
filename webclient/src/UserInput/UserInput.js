const UserInput = (props) => {
    const {handleChange, handleClick} = props
    return(
        <div>
            <label for="robot_command">Robot Command</label>
            <br></br>
            <input name="robot_command" onChange={handleChange}></input>
            <br></br>
            <button onClick={handleClick}>SEND</button>
        </div>
    );
};

export default UserInput;