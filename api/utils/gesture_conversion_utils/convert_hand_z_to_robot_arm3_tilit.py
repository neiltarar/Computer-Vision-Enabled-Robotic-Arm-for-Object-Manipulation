def round_to_nearest_multiple(number, multiple):
    return round(number / multiple) * multiple

def convert_hand_z_to_robot_arm3_angle(value, sensitivity=0, rounding_multiple=5):
    # Define the input range and output range
    input_min, input_max = 50, 90
    output_min, output_max = 0, 180

    # Clamp the input value to the input range
    value = max(min(value, input_max), input_min)

    # Normalize the input value to a 0 to 1 range
    normalized_value = (value - input_min) / (input_max - input_min)

    # Convert the normalized value to the output range
    output_value = output_min + normalized_value * (output_max - output_min)

    # Check if the output value is within the output range
    if output_value > max(output_max, output_min):
        output_value = max(output_max, output_min)
    elif output_value < min(output_max, output_min):
        output_value = min(output_max, output_min)

    # Round the output value to the desired sensitivity and multiple
    output_value = round(output_value, sensitivity)
    output_value = round_to_nearest_multiple(output_value, rounding_multiple)

    return output_value
