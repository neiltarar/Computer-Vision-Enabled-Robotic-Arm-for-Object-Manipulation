const UserInput = (props) => {
    const {handleChange, handleClick, handleKeyPress} = props
    return(
        <div>
            <label for="robot_command">Robot Command</label>
            <br></br>
            <input name="robot_command" onKeyDown={handleKeyPress} onChange={handleChange}></input>
            <br></br>
            <button onClick={handleClick}>SEND</button>
        </div>
    );
};

export default UserInput;