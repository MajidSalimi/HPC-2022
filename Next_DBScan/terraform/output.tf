output "instance_ip" {
  description = "The public ip for ssh access"
  value       = [aws_instance.hpc_ec2[0].public_ip, aws_instance.hpc_ec2[1].public_ip]
}