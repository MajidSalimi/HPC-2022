terraform {
  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = "3.74.1"
    }
  }

  required_version = ">= 0.14.9"
}

provider "aws" {
  profile = "default"
  region  = "eu-west-1"
}

resource "aws_key_pair" "hpc_ec2_ssh_key" {
  key_name   = "hpc_ec2_key"
  public_key = var.ec2_public_key
}

resource "aws_security_group" "ec2_security_group" {
  description = "Allow SSH connections"
  name        = "hpc_ec2_security_group"

  # Allow all traffic in ingress.
  # This is important for make MPI working. You have to open MPI ports.
  # In production environments is really discouraged to open all ports.
  ingress {
    description      = "All traffic"
    from_port        = 0
    to_port          = 0
    protocol         = "-1"
    cidr_blocks      = ["0.0.0.0/0"]
    ipv6_cidr_blocks = ["::/0"]
  }

  egress {
    description      = "All traffic"
    from_port        = 0
    to_port          = 0
    protocol         = "-1"
    cidr_blocks      = ["0.0.0.0/0"]
    ipv6_cidr_blocks = ["::/0"]
  }
}

resource "aws_instance" "hpc_ec2" {
  ami             = "ami-00c90dbdc12232b58"
  instance_type   = "t3.xlarge"
  count           = 4
  key_name        = "hpc_ec2_key"
  security_groups = ["${aws_security_group.ec2_security_group.name}"]
  depends_on      = [aws_security_group.ec2_security_group]
  tags = {
    Name = "hpc-${count.index}"
  }
}